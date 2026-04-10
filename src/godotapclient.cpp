#include "godotapclient.h"
#include "apclient.hpp"

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



void GodotAPClient::_init() {
    // initialize any variables here
}

void GodotAPClient::connect_to_host(String game, String uri) {
    std::string uuid = "e321d079-0848-4f83-8694-dc5d9bf2f764"; // TODO FIXME
    // TODO use ap_get_uuid(file, host)

    ap.reset(new APClient(uuid, game.utf8().get_data(), uri.utf8().get_data()));

    // set_socket_connected_handler
    ap->set_socket_connected_handler([this]() {
        emit_signal("socket_connected");
    });

    // set_socket_error_handler
    ap->set_socket_error_handler([this](const std::string& error) {
        emit_signal("socket_error", error.data());
    });

    // set_socket_disconnected_handler
    ap->set_socket_disconnected_handler([this]() {
        emit_signal("socket_disconnected");
    });

    // set_slot_connected_handler
    ap->set_slot_connected_handler([this](const nlohmann::json& json_data) {
        godot::Dictionary dict_data;

        // TODO FIXME I cannot be bothered to figure out APClient::json at 2:30AM but I know this quick fix is bad
        
        godot::String json_str = godot::String(json_data.dump().data());
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
            std::string str = *iter;

            reasons_array.append(godot::String(str.c_str()));

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
            godot::String gd_string = tags.read()[i];

            std::string cpp_string = gd_string.utf8().get_data();
        
            tags_list.push_back(cpp_string);
        }

        ap->ConnectSlot(name.utf8().get_data(), password.utf8().get_data(), items_handling, tags_list);
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

    std::string result = ap->get_player_alias(slot);
    return String(result.data());
}



const String GodotAPClient::get_player_game(int player) {
    if (!ap) { return String(""); }

    std::string result = ap->get_player_game(player);
    return String(result.data());
}



const String GodotAPClient::get_game() {
    if (!ap) { return String(""); }

    std::string result = ap->get_game();
    return String(result.data());
}



String GodotAPClient::get_location_name(int64_t code, const String& game) {
    if (!ap) { return String("Unknown"); }

    std::string result = ap->get_location_name(code, game.utf8().get_data());
    return String(result.data());
}



int64_t GodotAPClient::get_location_id(const String& name) const {
    if (!ap) { return APClient::INVALID_NAME_ID; }
    return ap->get_location_id(name.utf8().get_data());
}



String GodotAPClient::get_item_name(int64_t code, const String& game) {
    if (!ap) { return String("Unknown"); }

    std::string result = ap->get_item_name(code, game.utf8().get_data());
    return String  //String(result.c_str());
}



int64_t GodotAPClient::get_item_id(const String& name) const {
    if (!ap) { return APClient::INVALID_NAME_ID; }
    return ap->get_item_id(name.utf8().get_data());
}



bool GodotAPClient::slot_concerns_self(int slot) const {
    if (!ap) { return false; }
    return ap->slot_concerns_self(slot);
}



void GodotAPClient::send_say(const String& text) {
    if (!ap) { return; }
    ap->Say(text.utf8().get_data());
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