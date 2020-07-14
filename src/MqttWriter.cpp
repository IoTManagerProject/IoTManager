#include "MqttWriter.h"

#include "PrintMessage.h"
#include "Utils/StringUtils.h"

static const char* MODULE = "MqttWriter";

MqttWriter::MqttWriter(PubSubClient* client, const char* topic) : _client{client} {
    _topic = strdup(topic);
};

MqttWriter::~MqttWriter() {
    if (_topic) delete _topic;
}

bool MqttWriter::begin(unsigned int length) {
    if (!_client->connected()) {
        pm.error("not connected");
        return false;
    }
    _length = length;
    bool res = _client->beginPublish(_topic, length, false);
    if (!res) {
        pm.error("on publish");
    }

    return res;
}

bool MqttWriter::write(const char* data) {
    return _client->print(data);
}

bool MqttWriter::end() {
    pm.info("published: " + prettyBytes(_length));
    return _client->endPublish();
}
