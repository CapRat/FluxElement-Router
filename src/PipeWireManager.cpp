//
// Created by benji on 28.11.25.
//

#include "PipeWireManager.h"
#include <pipewire/extensions/security-context.h>
#include <pipewire/extensions/metadata.h>
#include <iostream>
#include <chrono>
#include "PipeWire/Loop.h"

struct ProxySupportStruct {
    pw_proxy *linkProxy = nullptr;
    spa_hook hook{};

    ~ProxySupportStruct() = default;
};

struct PipeWire::PipeWireManager::PipeWireData {
    std::unique_ptr<Loop> mainLoop = nullptr;
    std::mutex mutex;
};

PipeWire::PipeWireManager::PipeWireManager(
    const PipeWireInitialised &pipewireInitialized,
    const PipeWireChangedCallbackType &pipewireChangedCallback) {
    // Initialise Internal DataStructure
    this->setPipewireInitialisedCallback(pipewireInitialized);
    this->setPipewireHasChangedCallback(pipewireChangedCallback);

    pipeWireData = std::make_unique<PipeWireData>();
    pw_init(nullptr, nullptr);
    pipeWireData->mainLoop = std::make_unique<Loop>();
    pipeWireData->mainLoop->getContext()->getCore()->getRegistry()->global += [this
            ](ID id, Permission permiossions, EventType type, Version version, Props props) {
                this->pipewireEntityAdded(id, permiossions, type, props);
            };
    pipeWireData->mainLoop->getContext()->getCore()->getRegistry()->initialized += [this]() {
        this->pipewireInitialized();
    };
    pipeWireData->mainLoop->start();
}


PipeWire::PipeWireManager::~PipeWireManager() {
    pw_deinit();
}

std::vector<PipeWire::Node> PipeWire::PipeWireManager::listNodes() {
    std::scoped_lock lock(pipeWireData->mutex);
    return nodes;
}

std::vector<PipeWire::Link> PipeWire::PipeWireManager::listLinks() {
    std::scoped_lock lock(pipeWireData->mutex);
    return links;
}

void PipeWire::PipeWireManager::setPipewireHasChangedCallback(
    const PipeWireChangedCallbackType &pipewireHasChangedCallback) {
    this->pipewireChangedCallback = pipewireHasChangedCallback;
}

void PipeWire::PipeWireManager::setPipewireInitialisedCallback(const PipeWireInitialised &pipeWireInitialised) {
    this->pipewireInitialized = pipeWireInitialised;
}

PipeWire::Node *PipeWire::PipeWireManager::getNodeWithId(NodeId entityId) {
    std::scoped_lock lock(this->pipeWireData->mutex);
    auto foundNode = std::ranges::find_if(nodes, [entityId](const Node &node) {
        return node.id == entityId;
    });
    if (foundNode != nodes.end()) {
        return &*foundNode; // pointer to the element
    }
    return nullptr;
}

PipeWire::Link *PipeWire::PipeWireManager::getLinkWithPorts(PortId output, PortId input) {
    std::scoped_lock lock(this->pipeWireData->mutex);
    auto foundLink = std::ranges::find_if(links, [input,output](const Link &l) {
        return l.inputPort == input && l.outputPort == output;
    });
    if (foundLink != links.end()) {
        return &*foundLink; // pointer to the element
    }
    return nullptr;
}

void PipeWire::PipeWireManager::pipewireEntityAdded(ID entityId, Permission permissions, const EventType &type,
                                                    Props props) {
    if (type == PW_TYPE_INTERFACE_Node) {
        Node node;
        node.id = entityId;
        node.name = props[PW_KEY_NODE_NAME];
        node.mediaClass = props[PW_KEY_MEDIA_CLASS];
        node.description = props[PW_KEY_NODE_DESCRIPTION];
        node.nickname = props[PW_KEY_NODE_NICK];
        node.ports = {};
        std::scoped_lock lock(this->pipeWireData->mutex);
        this->nodes.push_back(node);
        pipeWireHasChanged(EntityType::Node);
    } else if (type == PW_TYPE_INTERFACE_Port) {
        Port port;
        port.id = entityId;
        const auto node = getNodeWithId(std::stoi(props[PW_KEY_NODE_ID]));
        port.name = props[PW_KEY_PORT_NAME];
        port.alias = props[PW_KEY_PORT_ALIAS];
        port.direction = props[PW_KEY_PORT_DIRECTION];
        node->ports.push_back(port);
        pipeWireHasChanged(EntityType::Port);
    } else if (type == PW_TYPE_INTERFACE_Link) {
        Link link;
        link.id = entityId;
        link.inputPort = std::stoi(props[PW_KEY_LINK_INPUT_PORT]);
        link.outputPort = std::stoi(props[PW_KEY_LINK_OUTPUT_PORT]);
        link.inputNode = this->getParentNodeFromPort(link.inputPort);
        link.outputNode = this->getParentNodeFromPort(link.outputPort);
        link.impl = static_cast<pw_proxy *>(pw_registry_bind(
            pipeWireData->mainLoop->getContext()->getCore()->getRegistry()->get(),
            link.id,
            PW_TYPE_INTERFACE_Link,
            PW_VERSION_LINK,
            0
        ));
        if (!this->getLinkWithPorts(link.outputPort, link.inputPort)) {

            this->links.push_back(link);
            pipeWireHasChanged(EntityType::Link);
        }
    } else if (type == PW_TYPE_INTERFACE_Client) {
        auto pidStr = props[PW_KEY_SEC_PID];
        auto clientName = props[PW_KEY_APP_NAME];
        if (std::stoi(pidStr) == getpid()) {
            /*   // Setting permissions on remove pipewire object (doesnt work though)
               auto *client = static_cast<pw_client *>(pw_registry_bind(
                   pipeWireData->loop,
                   entityId,
                   PW_TYPE_INTERFACE_Client,
                   PW_VERSION_CLIENT,
                   0
               ));

               pipeWireData->clientProxy = client;
               pw_permission perm = PW_PERMISSION_INIT(PW_ID_ANY, PW_PERM_ALL);
               //pw_core_sync(pipeWireData->core, PW_ID_CORE, 0);
               */
        }
    }
    else if (type == PW_TYPE_INTERFACE_SecurityContext) {
        auto x=5;

    }
    else if (type == PW_TYPE_INTERFACE_Metadata) {

      auto y =2;
    }
}

void PipeWire::PipeWireManager::pipewireEntityRemoved(uint32_t entityId) {
    // Do deletion of node here

    std::scoped_lock lock(this->pipeWireData->mutex);
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
    return this->pipeWireData->mainLoop->invokeFunctionInLoop([this, portOutId, portInId]() {
        return this->internalConnectPorts(portOutId, portInId) == 0;
    }) == 0;
}


bool PipeWire::PipeWireManager::internalConnectPorts(const PortId portOutId, const PortId portInId) {
    const auto portInIdStr = std::to_string(portInId);
    const auto portOutIdStr = std::to_string(portOutId);
    const spa_dict_item props_items[] = {
        {PW_KEY_LINK_OUTPUT_PORT, portOutIdStr.c_str()},
        {PW_KEY_LINK_INPUT_PORT, portInIdStr.c_str()},
        {PW_KEY_ACCESS, "0x1f"} // R W X M D
    };
    const auto props = SPA_DICT_INIT_ARRAY(props_items);

    auto *linkProxy = static_cast<pw_proxy *>(pw_core_create_object(
        pipeWireData->mainLoop->getContext()->getCore()->get(),
        "link-factory",
        PW_TYPE_INTERFACE_Link,
        PW_VERSION_LINK,
        &props,
        0
    ));
    if (!linkProxy) {
        return false;
    }

    // pw_proxy_add_listener(linkProxy,&pipeWireData->proxyListener,&pipeWireData->proxy_events, this);
    pw_core_sync(pipeWireData->mainLoop->getContext()->getCore()->get(),PW_ID_CORE, 0);
    Link l;
    l.id = pw_proxy_get_id(linkProxy);
    l.inputPort = portInId;
    l.outputPort = portOutId;
    l.impl = linkProxy;
    this->links.push_back(l);
    pipeWireHasChanged(EntityType::Link);
    return true;
}

bool PipeWire::PipeWireManager::disconnectPorts(PortId portOutId, PortId portInId) {
    return this->pipeWireData->mainLoop->invokeFunctionInLoop([this, portOutId, portInId]() {
        return this->internalDisconnectPorts(portOutId, portInId) == 0;
    }) == 0;
}

bool PipeWire::PipeWireManager::internalDisconnectPorts(PortId portOutId, PortId portInId) {
    auto link = getLinkWithPorts(portOutId, portInId);
    if (link == nullptr)return false;

    pw_proxy_destroy(static_cast<pw_proxy *>(link->impl));
    return true;
}

PipeWire::NodeId PipeWire::PipeWireManager::getParentNodeFromPort(PortId portId) {
    std::scoped_lock lock(this->pipeWireData->mutex);
    for (const auto &node: nodes) {
        for (const auto &port: node.ports) {
            if (portId == port.id) {
                return node.id;
            }
        }
    }
    return -1;
}


void PipeWire::PipeWireManager::pipeWireHasChanged(EntityType type) {
    if (this->pipewireChangedCallback != nullptr) {
        this->pipewireChangedCallback(type);
    }
}

void PipeWire::PipeWireManager::pipeWireInitialised() {
    if (this->pipewireInitialized != nullptr) {
        this->pipewireInitialized();
    }
}


