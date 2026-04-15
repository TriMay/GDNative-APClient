#include "godotapclient.h"
#include "apclient.hpp"
#include <iostream>

using namespace godot;



void GodotAPClient::_register_methods() {
    register_method("connect_to_host", &GodotAPClient::connect_to_host);
    register_method("poll", &GodotAPClient::poll);
    register_method("connect_slot", &GodotAPClient::connect_slot);

    register_method("get_player_alias", &GodotAPClient::get_player_alias);
    register_method("get_player_game", &GodotAPClient::get_player_game);
    register_method("get_game", &GodotAPClient::get_game);
    register_method("get_location_name", &GodotAPClient::get_location_name);
    register_method("get_location_id", &GodotAPClient::get_location_id);
    register_method("get_item_name", &GodotAPClient::get_item_name);
    register_method("get_item_id", &GodotAPClient::get_item_id);
    register_method("slot_concerns_self", &GodotAPClient::slot_concerns_self);

    register_method("send_say", &GodotAPClient::send_say);
    register_method("send_location_checks", &GodotAPClient::send_location_checks);
    register_method("send_location_scouts", &GodotAPClient::send_location_scouts);


    register_signal<GodotAPClient>("socket_connected");
    register_signal<GodotAPClient>("socked_disconnected");
    register_signal<GodotAPClient>("socket_error",
            "error", GODOT_VARIANT_TYPE_STRING);
    register_signal<GodotAPClient>("slot_connected",
            "slot_data", GODOT_VARIANT_TYPE_DICTIONARY);
    register_signal<GodotAPClient>("slot_refused",
            "reasons", GODOT_VARIANT_TYPE_POOL_STRING_ARRAY);
    register_signal<GodotAPClient>("slot_disconnected");
    register_signal<GodotAPClient>("room_info");
    register_signal<GodotAPClient>("room_update");
    register_signal<GodotAPClient>("recieved_item",
            "index", GODOT_VARIANT_TYPE_INT,
            "item", GODOT_VARIANT_TYPE_INT,
            "location", GODOT_VARIANT_TYPE_INT,
            "player", GODOT_VARIANT_TYPE_INT,
            "flags", GODOT_VARIANT_TYPE_INT);
    register_signal<GodotAPClient>("location_info",
            "index", GODOT_VARIANT_TYPE_INT,
            "item", GODOT_VARIANT_TYPE_INT,
            "location", GODOT_VARIANT_TYPE_INT,
            "player", GODOT_VARIANT_TYPE_INT,
            "flags", GODOT_VARIANT_TYPE_INT);
}

GodotAPClient::GodotAPClient() {
}

GodotAPClient::~GodotAPClient() {
    // add your cleanup here
}


inline godot::String GodotAPClient::std_to_godotstr(const std::string &input) const {
    const char * c_str = input.c_str();
    return godot::String(c_str);
}

inline std::string GodotAPClient::godot_to_stdstr(const godot::String &input) const {
    godot::CharString gdchar_str = input.utf8();
    const char * c_str = gdchar_str.get_data();
    return std::string(c_str);
}



void GodotAPClient::_init() {
    // initialize any variables here
}

void GodotAPClient::connect_to_host(String game, String uri) {
    std::string uuid = "e321d079-0848-4f83-8694-dc5d9bf2f764"; // TODO FIXME
    // TODO use ap_get_uuid(file, host)
    
    std::string game_as_std = godot_to_stdstr(game);
    std::string uri_as_std = godot_to_stdstr(uri);

    ap.reset(new APClient(uuid, game_as_std, uri_as_std));

    // set_socket_connected_handler
    ap->set_socket_connected_handler([this]() {
        emit_signal("socket_connected");
    });

    // set_socket_error_handler
    ap->set_socket_error_handler([this](const std::string& error) {
        emit_signal("socket_error", std_to_godotstr(error));
    });

    // set_socket_disconnected_handler
    ap->set_socket_disconnected_handler([this]() {
        emit_signal("socket_disconnected");
    });

    // set_slot_connected_handler
    ap->set_slot_connected_handler([this](const nlohmann::json& json_data) {
        godot::Dictionary dict_data;

        // TODO FIXME I cannot be bothered to figure out APClient::json at 2:30AM but I know this quick fix is bad
        
        std::string json_dump = json_data.dump();
        godot::String json_str = std_to_godotstr(json_dump);
        godot::Ref<godot::JSONParseResult> parse_result = godot::JSON::get_singleton()->parse(json_str);
        
        if (parse_result->get_error() == godot::Error::OK) {
            dict_data = parse_result->get_result();
        }
        // TODO else report error

        emit_signal("slot_connected", dict_data);
    });

    // set_slot_refused_handler
    ap->set_slot_refused_handler([this](const std::list<std::string>& reasons_list) {
        godot::PoolStringArray reasons_array;

        std::list<std::string>::const_iterator iter = reasons_list.begin();

        for (int i = 0; i < reasons_list.size(); i++) {
            std::string std_str = *iter;
            String gd_str = std_to_godotstr(std_str);

            reasons_array.append(gd_str);

            ++iter;
        }
        emit_signal("slot_refused", reasons_array);
    });

    // set_slot_disconnected_handler
    ap->set_slot_disconnected_handler([this]() {
        emit_signal("slot_disconnected");
    });
    
    // set_room_info_handler
    ap->set_room_info_handler([this]() {
        emit_signal("room_info");
        // use functions to get room info
    });

    // set_room_update_handler
    ap->set_room_update_handler([this]() {
        emit_signal("room_update");
        // use functions to get room info
    });

    // set_items_received_handler
    ap->set_items_received_handler([this](const std::list<APClient::NetworkItem>& item_list) {
        std::list<APClient::NetworkItem>::const_iterator iter = item_list.begin();

        for (int i = 0; i < item_list.size(); i++) {
            APClient::NetworkItem item = *iter;

            emit_signal("recieved_item", item.index, item.item, item.location, item.player, item.flags);

            ++iter;
        }
    });

    // set_location_info_handler
    ap->set_location_info_handler([this](const std::list<APClient::NetworkItem>&item_list) {
        std::list<APClient::NetworkItem>::const_iterator iter = item_list.begin();

        for (int i = 0; i < item_list.size(); i++) {
            APClient::NetworkItem item = *iter;

            emit_signal("location_info", item.index, item.item, item.location, item.player, item.flags);

            ++iter;
        }
    }); // TODO test this after LocationScout

    // set_data_package_changed_handler

    // set_print_handler

    // set_print_json_handler

    // set_bounced_handler

    // set_location_checked_handler

    // set_retrieved_handler

    // set_set_reply_handler
}

void GodotAPClient::poll() {
    if (ap) ap->poll();
}



void GodotAPClient::connect_slot(const String& name, const String& password, int items_handling, PoolStringArray tags) {
    if (ap) {
        std::list<std::string> tags_list;

        for (int i = 0; i < tags.size(); i++) {
            godot::String gd_str = tags.read()[i];

            std::string std_str = godot_to_stdstr(gd_str);
        
            tags_list.push_back(std_str);
        }

        std::string name_as_std = godot_to_stdstr(name);
        std::string pass_as_std = godot_to_stdstr(password);

        ap->ConnectSlot(name_as_std, pass_as_std, items_handling, tags_list);
    }
}


// TODO FIXME
//void GodotAPClient::connect_update() {
//    if (ap) {
//        ap->ConnectUpdate(send_items_handling, items_handling, send_tags, tags_list)
//    }
//}


String GodotAPClient::get_player_alias(int slot) {
    if (!ap) { return String("Unknown"); }

    std::string std_str = ap->get_player_alias(slot);
    return std_to_godotstr(std_str);
}



const String GodotAPClient::get_player_game(int player) {
    if (!ap) { return String(""); }

    std::string std_str = ap->get_player_game(player);
    return std_to_godotstr(std_str);
}



const String GodotAPClient::get_game() {
    if (!ap) { return String(""); }

    std::string std_str = ap->get_game();
    return std_to_godotstr(std_str);
}



String GodotAPClient::get_location_name(int64_t code, const String& game) {
    if (!ap) { return String("Unknown"); }

    std::string std_str = ap->get_location_name(code, godot_to_stdstr(game));
    return std_to_godotstr(std_str);
}



int64_t GodotAPClient::get_location_id(const String& name) const {
    if (!ap) { return APClient::INVALID_NAME_ID; }

    return ap->get_location_id(godot_to_stdstr(name));
}



String GodotAPClient::get_item_name(int64_t code, const String& game) {
    if (!ap) { return String("Unknown"); }

    std::string std_str = ap->get_item_name(code, godot_to_stdstr(game));
    return std_to_godotstr(std_str);
}



int64_t GodotAPClient::get_item_id(const String& name) const {
    if (!ap) { return APClient::INVALID_NAME_ID; }

    return ap->get_item_id(godot_to_stdstr(name));
}



bool GodotAPClient::slot_concerns_self(int slot) const {
    if (!ap) { return false; }
    return ap->slot_concerns_self(slot);
}



void GodotAPClient::send_say(const String& text) {
    if (!ap) { return; }
    ap->Say(godot_to_stdstr(text));
}



void GodotAPClient::send_location_checks(Array locations_array) {
    // NOTE We don't use PoolIntArray due to that being limited to 32-bit int's, despite Godot's int type being 64 bits
    if (ap) {
        std::list<int64_t> locations_list;

        for (int i = 0; i < locations_array.size(); i++) {
            Variant location = locations_array[i];
            if (location.get_type() == Variant::Type::INT) {
                locations_list.push_back((int64_t)location);
            }
        }

        ap->LocationChecks(locations_list);
    }
}



void GodotAPClient::send_location_scouts(Array locations_array, int create_as_hint) {
    // NOTE We don't use PoolIntArray due to that being limited to 32-bit int's, despite Godot's int type being 64 bits
    if (ap) {
        std::list<int64_t> locations_list;

        for (int i = 0; i < locations_array.size(); i++) {
            Variant location = locations_array[i];
            if (location.get_type() == Variant::Type::INT) {
                locations_list.push_back((int64_t)location);
            }
        }

        ap->LocationScouts(locations_list, create_as_hint);
    }
}