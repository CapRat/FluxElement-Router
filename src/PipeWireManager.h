#pragma once
#include <pipewire/pipewire.h>
#include <spa/utils/result.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <algorithm>

namespace PipeWire {
    using PortId = uint32_t;
    using NodeId = uint32_t;
    using LinkId = uint32_t;

    struct Port {
        PortId id{};
        std::string name;
        std::string alias;
        std::string direction;
    };

    struct Node {
        NodeId id{};
        std::string name;
        std::string mediaClass; // "Audio/Source", "Audio/Sink", "Midi/Source", "Midi/Sink"
        std::string description;
        std::string nickname;
        std::vector<Port> ports;
    };

    struct Link {
        LinkId id{};
        PortId inputPort{};
        PortId outputPort{};
        pw_proxy * proxyRef=nullptr;
    };

    class PipeWireManager {
    public:
        enum class EntityType {
            Node, Port, Link
        };
        using PipeWireChangedCallbackType = std::function<void(EntityType)>;
        PipeWireManager(const PipeWireChangedCallbackType& pipewireChangedCallback=nullptr);

        ~PipeWireManager();

        // Thread-safe copy of node list
        std::vector<Node> listNodes();
        // Thread.sage copy of Links
        std::vector<Link> listLinks();

        void setPipewireHasChangedCallback(const PipeWireChangedCallbackType &pipewireHasChangedCallback);

        // Connect two nodes (simplified link creation)
        bool connectPorts( PortId portOutId, PortId portInId);

        // Disconnect (remove first matching link – simplified)
        bool disconnectPorts(PortId outputNode, PortId inputNode);

    private:
        Node *getNodeWithId(NodeId entityId);
        Link *getLinkWithPorts(PortId output, PortId input);
        void pipewireEntityAdded(uint32_t entityId,const std::string &type, const spa_dict *props);
        void pipewireEntityRemoved(uint32_t entityId);
        [[nodiscard]] bool internalConnectPorts(PortId portOutId, PortId portInId) const;
        [[nodiscard]] bool internalDisconnectPorts(PortId portOutId, PortId portInId);
        void pipeWireHasChanged(EntityType type);

        pw_main_loop *loop;
        pw_context *context;
        pw_core *core;
        pw_registry *registry;
        spa_hook registryListener{};
        std::thread loopThread;
        bool running;
        std::mutex mutex;


        std::vector<Node> nodes;
        std::vector<Link> links;

        PipeWireChangedCallbackType pipewireChangedCallback;
        pw_registry_events registryEvents{};
    };
}
