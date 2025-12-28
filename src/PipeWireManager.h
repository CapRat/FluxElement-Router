#pragma once
#include <pipewire/pipewire.h>
#include <spa/utils/result.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <algorithm>

#include "PipeWire/Registry.h"

namespace PipeWire {
    using PortId = uint32_t;
    using NodeId = uint32_t;
    using LinkId = uint32_t;

    struct Port {
        PortId id{};
        std::string name;
        std::string alias;
        std::string direction;
        void* impl=nullptr;
    };

    struct Node {
        NodeId id{};
        std::string name;
        std::string mediaClass; // "Audio/Source", "Audio/Sink", "Midi/Source", "Midi/Sink"
        std::string description;
        std::string nickname;
        std::vector<Port> ports;
        void* impl=nullptr;
    };

    struct Link {
        LinkId id{};
        PortId inputPort{};
        PortId outputPort{};
        pw_proxy *proxyRef = nullptr;
        NodeId inputNode{};
        NodeId outputNode{};
        void* impl=nullptr;
    };

    class PipeWireManager {
    public:
        enum class EntityType {
            Node, Port, Link
        };

        using PipeWireChangedCallbackType = std::function<void(EntityType)>;
        using PipeWireInitialised= std::function<void()>;
        explicit PipeWireManager(const PipeWireInitialised &pipewireInitialized=nullptr, const PipeWireChangedCallbackType &pipewireChangedCallback = nullptr);

        ~PipeWireManager();

        // Thread-safe copy of node list
        std::vector<Node> listNodes();

        // Thread.sage copy of Links
        std::vector<Link> listLinks();

        void setPipewireHasChangedCallback(const PipeWireChangedCallbackType &pipewireHasChangedCallback);
        void setPipewireInitialisedCallback(const PipeWireInitialised &pipeWireInitialised);
        // Connect two nodes (simplified link creation)
        bool connectPorts(PortId portOutId, PortId portInId);

        // Disconnect (remove first matching link – simplified)
        bool disconnectPorts(PortId outputNode, PortId inputNode);

    private:
        NodeId getParentNodeFromPort(PortId portId);
        Node *getNodeWithId(NodeId entityId);
        Link *getLinkWithPorts(PortId output, PortId input);
        void pipewireEntityAdded(ID entityId, Permission permissions, const EventType &type,
                                 Props props);
        void pipewireEntityRemoved(uint32_t entityId);
        [[nodiscard]] bool internalConnectPorts(PortId portOutId, PortId portInId);
        [[nodiscard]] bool internalDisconnectPorts(PortId portOutId, PortId portInId);
        void pipeWireHasChanged(EntityType type);
        void pipeWireInitialised();
        struct PipeWireData;
        std::unique_ptr<PipeWireData> pipeWireData;
        std::vector<Node> nodes;
        std::vector<Link> links;

        PipeWireInitialised pipewireInitialized;
        PipeWireChangedCallbackType pipewireChangedCallback;


    };
}
