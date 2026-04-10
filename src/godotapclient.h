#ifndef GODOTAPCLIENT_H
#define GODOTAPCLIENT_H


#include <inttypes.h>
#include <string>

#include "map"

#include "Godot.hpp"
#include "Object.hpp"
#include "OS.hpp"
#include "Engine.hpp"
#include "Texture.hpp"
#include "String.hpp"
#include "Reference.hpp"
#include "Dictionary.hpp"
#include "IP.hpp"
#include "Variant.hpp"
#include "JSON.hpp"
#include "JSONParseResult.hpp"


#include "apclient.hpp"
// include "apuuid.hpp" TODO FIXME for some reason this compile errors



namespace godot {

    class GodotAPClient : public Object {
        GODOT_CLASS(GodotAPClient, Object)

    public:
        static void _register_methods();

        GodotAPClient();
        ~GodotAPClient();

        void _init();

        void connect_to_host(String game, String url);

        void poll();

        void connect_slot(const String& name, const String& password, int items_handling, PoolStringArray tags);


        // get_checked_locations

        // get_missing_locations

        // get_players

        String get_player_alias(int slot);

        const String get_player_game(int player);

        const String get_game();

        String get_location_name(int64_t code, const String& game);

        int64_t get_location_id(const String& name) const;

        String get_item_name(int64_t code, const String& game);

        int64_t get_item_id(const String& name) const;

        bool slot_concerns_self(int slot) const;

        // render_json


        void send_say(const String& text);

        void send_location_checks(Array locations_array);

        void send_location_scouts(Array locations_array, int create_as_hint);

        //void connect_update();


    private:
        std::unique_ptr<APClient> ap;
    };

}




#endif