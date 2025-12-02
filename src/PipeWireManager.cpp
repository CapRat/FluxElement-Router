//
// Created by benji on 28.11.25.
//

#include "PipeWireManager.h"

#include <iostream>

struct TransportManPortPort {
    PipeWire::PipeWireManager *manager;
    PipeWire::PortId portOut;
    PipeWire::PortId portIn;
};

PipeWire::PipeWireManager::PipeWireManager(const PipeWireChangedCallbackType &pipewireChangedCallback) : loop(nullptr),
    context(nullptr), core(nullptr), registry(nullptr),
    running(true) {
    this->setPipewireHasChangedCallback(pipewireChangedCallback);
    pw_init(nullptr, nullptr);

    loop = pw_main_loop_new(nullptr);
    context = pw_context_new(pw_main_loop_get_loop(loop), nullptr, 0);
    //pw_context_load_module(context, "libpipewire-module-link-factory", NULL, NULL);
    core = pw_context_connect(context, nullptr, 0);

    if (!core)
        throw std::runtime_error("Failed to connect to PipeWire core");

    registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
    if (!registry)
        throw std::runtime_error("Failed to get PipeWire registry");

    registryEvents = {
        PW_VERSION_REGISTRY_EVENTS,
        [](void *data,
           const uint32_t id,
           uint32_t permissions,
           const char *type,
           uint32_t version,
           const spa_dict *props) {
            auto *self = static_cast<PipeWireManager *>(data);
            self->pipewireEntityAdded(id, std::string(type), props);
        },
        [](void *data, uint32_t id) {
            auto *self = static_cast<PipeWireManager *>(data);
            self->pipewireEntityRemoved(id);
        }

    };


    pw_registry_add_listener(
        registry,
        &registryListener,
        &registryEvents,
        this
    );

    // Start background event loop thread
    loopThread = std::thread([this]() {
        pw_main_loop_run(loop);
    });
}


PipeWire::PipeWireManager::~PipeWireManager() {
    running = false;

    if (loop)
        pw_main_loop_quit(loop);

    if (loopThread.joinable())
        loopThread.join();


    if (registry) pw_proxy_destroy((pw_proxy *) registry);
    if (core) pw_core_disconnect(core);
    if (context) pw_context_destroy(context);
    if (loop) pw_main_loop_destroy(loop);

    pw_deinit();
}

std::vector<PipeWire::Node> PipeWire::PipeWireManager::listNodes() {
    std::scoped_lock lock(mutex);
    return nodes;
}

std::vector<PipeWire::Link> PipeWire::PipeWireManager::listLinks() {
    std::scoped_lock lock(mutex);
    return links;
}

void PipeWire::PipeWireManager::setPipewireHasChangedCallback(
    const PipeWireChangedCallbackType &pipewireHasChangedCallback) {
    this->pipewireChangedCallback = pipewireHasChangedCallback;
}

PipeWire::Node *PipeWire::PipeWireManager::getNodeWithId(NodeId entityId) {
    std::scoped_lock lock(this->mutex);
    auto foundNode = std::ranges::find_if(nodes, [entityId](const Node &node) {
        return node.id == entityId;
    });
    if (foundNode != nodes.end()) {
        return &*foundNode; // pointer to the element
    }
    return nullptr;
}

PipeWire::Link *PipeWire::PipeWireManager::getLinkWithPorts(PortId output, PortId input) {
    std::scoped_lock lock(this->mutex);
    auto foundLink = std::ranges::find_if(links, [input,output](const Link &l) {
        return l.inputPort == input && l.outputPort == output;
    });
    if (foundLink != links.end()) {
        return &*foundLink; // pointer to the element
    }
    return nullptr;
}

void PipeWire::PipeWireManager::pipewireEntityAdded(uint32_t entityId, const std::string &type, const spa_dict *props) {
    auto getFromSpaDict = [&](const char *k) {
        const char *v = spa_dict_lookup(props, k);
        return v ? std::string(v) : "";
    };
    if (type == PW_TYPE_INTERFACE_Node) {
        Node node;
        node.id = entityId;
        node.name = getFromSpaDict(PW_KEY_NODE_NAME);
        node.mediaClass = getFromSpaDict(PW_KEY_MEDIA_CLASS);
        node.description = getFromSpaDict(PW_KEY_NODE_DESCRIPTION);
        node.nickname = getFromSpaDict(PW_KEY_NODE_NICK);
        node.ports = {};

        std::scoped_lock lock(this->mutex);

        this->nodes.push_back(node);
        pipeWireHasChanged(EntityType::Node);
    } else if (type == PW_TYPE_INTERFACE_Port) {
        Port port;
        port.id = entityId;
        const auto node = getNodeWithId(std::stoi(getFromSpaDict(PW_KEY_NODE_ID)));
        port.name = getFromSpaDict(PW_KEY_PORT_NAME);
        port.alias = getFromSpaDict(PW_KEY_PORT_ALIAS);
        port.direction = getFromSpaDict(PW_KEY_PORT_DIRECTION);
        node->ports.push_back(port);
        pipeWireHasChanged(EntityType::Port);
    } else if (type == PW_TYPE_INTERFACE_Link) {
        Link link;
        link.id = entityId;
        link.inputPort = std::stoi(getFromSpaDict(PW_KEY_LINK_INPUT_PORT));
        link.outputPort = std::stoi(getFromSpaDict(PW_KEY_LINK_OUTPUT_PORT));
        link.inputNode= this->getParentNodeFromPort(link.inputPort);
        link.outputNode= this->getParentNodeFromPort(link.outputPort);
        auto str=  getFromSpaDict(PW_KEY_NODE_NAME);
        this->links.push_back(link);
        pipeWireHasChanged(EntityType::Link);
    }
}

void PipeWire::PipeWireManager::pipewireEntityRemoved(uint32_t entityId) {
    // Do deletion of node here

    std::scoped_lock lock(this->mutex);
    for (auto nIt = nodes.begin(); nIt != nodes.end();) {
        // 1) Remove node if its ID matches
        if (nIt->id == entityId) {
            nIt = nodes.erase(nIt);
            pipeWireHasChanged(EntityType::Node);
            return;
        }

        // 2) Otherwise scan its ports
        auto &ports = nIt->ports;
        for (auto pIt = ports.begin(); pIt != ports.end();) {
            if (pIt->id == entityId) {
                pIt = ports.erase(pIt);
                pipeWireHasChanged(EntityType::Port);
                return;
            } else {
                ++pIt;
            }
        }

        ++nIt; // move to next node
    }
    // 3) Remove links
    for (auto lIt = links.begin(); lIt != links.end();) {
        if (lIt->id == entityId) {
            lIt = links.erase(lIt);
            pipeWireHasChanged(EntityType::Link);
            return;
        }
    }
}

bool PipeWire::PipeWireManager::connectPorts(const PortId portOutId, const PortId portInId) {
    auto transferObj = new std::tuple{this, portOutId, portInId};
    auto retCode = pw_loop_invoke(
        pw_main_loop_get_loop(loop),
        [](spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data)-> int {
            //  const auto transferObj=static_cast<const TransportManPortPort*>(user_data);
            auto transferObj = static_cast<std::tuple<PipeWireManager *, PortId, PortId> *>(user_data);
            auto self = std::get<0>(*transferObj);
            auto retVal = self->internalConnectPorts(std::get<1>(*transferObj), std::get<2>(*transferObj));
            delete transferObj;
            return retVal;
        },
        0,
        nullptr,
        0,
        false,
        transferObj
    );

    return retCode == 0;
}


bool PipeWire::PipeWireManager::internalConnectPorts(const PortId portOutId, const PortId portInId) const {
    const auto portInIdStr = std::to_string(portInId);
    const auto portOutIdStr = std::to_string(portOutId);
    const spa_dict_item props_items[] = {
        {PW_KEY_LINK_OUTPUT_PORT, portOutIdStr.c_str()},
        {PW_KEY_LINK_INPUT_PORT, portInIdStr.c_str()},
    };
    const auto props = SPA_DICT_INIT_ARRAY(props_items);

    auto *linkProxy = static_cast<pw_proxy *>(pw_core_create_object(
        core,
        "link-factory",
        PW_TYPE_INTERFACE_Link,
        PW_VERSION_LINK,
        &props,
        0
    ));
    if (!linkProxy) {
        return false;
    }
    pw_core_sync(core,PW_ID_CORE, 0);
    return true;
}

bool PipeWire::PipeWireManager::disconnectPorts(PortId portOutId, PortId portInId) {
    auto transferObj = new std::tuple{this, portOutId, portInId};
    auto retCode = pw_loop_invoke(
        pw_main_loop_get_loop(loop),
        [](struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size,
           void *user_data)-> int {
            auto transferObj = static_cast<std::tuple<PipeWireManager *, PortId, PortId> *>(user_data);
            auto self = std::get<0>(*transferObj);
            auto retVal = self->internalDisconnectPorts(std::get<1>(*transferObj), std::get<2>(*transferObj));
            delete transferObj;
            return retVal;
        },
        0,
        nullptr,
        0,
        false,
        transferObj
    );

    return retCode == 0;
}

PipeWire::NodeId PipeWire::PipeWireManager::getParentNodeFromPort(PortId portId) {
    std::scoped_lock lock(this->mutex);
    for (const auto& node:nodes) {
        for (const auto& port:node.ports) {
            if(portId==port.id) {
                return node.id;
            }
        }
    }
    return -1;
}

bool PipeWire::PipeWireManager::internalDisconnectPorts(PortId portOutId, PortId portInId) {
    auto link = getLinkWithPorts(portOutId, portInId);
    auto *linkProxy = static_cast<pw_proxy *>(pw_registry_bind(
        registry,
        link->id,
        PW_TYPE_INTERFACE_Link,
        PW_VERSION_LINK,
        0
    ));
    if (!linkProxy) {
        return false;
    }

    pw_proxy_destroy(linkProxy); // this deletes the link and disconnects ports
    return true;
}

void PipeWire::PipeWireManager::pipeWireHasChanged(EntityType type) {
    if (this->pipewireChangedCallback != nullptr) {
        this->pipewireChangedCallback(type);
    }
}


