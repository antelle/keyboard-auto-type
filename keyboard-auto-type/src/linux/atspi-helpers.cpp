#include "atspi-helpers.h"

#include <atspi/atspi.h>

#include <algorithm>
#include <vector>

namespace keyboard_auto_type {

std::string get_browser_url_using_atspi(pid_t pid) {
    if (!atspi_is_initialized()) {
        if (atspi_init()) {
            return "";
        }
    }

    std::vector<GObject *> to_free;

    std::string url;

    auto desktops_count = atspi_get_desktop_count();
    for (auto desktop_ix = 0; desktop_ix < desktops_count; desktop_ix++) {
        auto *desktop = atspi_get_desktop(desktop_ix);
        if (!desktop) {
            continue;
        }
        to_free.push_back(reinterpret_cast<GObject *>(desktop));

        auto windows_count = atspi_accessible_get_child_count(desktop, nullptr);
        for (auto window_ix = 0; window_ix < windows_count; window_ix++) {
            auto *window = atspi_accessible_get_child_at_index(desktop, window_ix, nullptr);
            if (!window) {
                continue;
            }
            to_free.push_back(reinterpret_cast<GObject *>(window));

            auto window_pid = atspi_accessible_get_process_id(window, nullptr);
            if (static_cast<pid_t>(window_pid) != pid) {
                continue;
            }

            auto tab_count = atspi_accessible_get_child_count(window, nullptr);
            for (auto tab_ix = 0; tab_ix < tab_count; tab_ix++) {
                auto *tab = atspi_accessible_get_child_at_index(window, tab_ix, nullptr);
                if (!tab) {
                    continue;
                }
                to_free.push_back(reinterpret_cast<GObject *>(tab));

                auto *state_set = atspi_accessible_get_state_set(tab);
                to_free.push_back(reinterpret_cast<GObject *>(state_set));
                auto is_active = atspi_state_set_contains(state_set, ATSPI_STATE_ACTIVE);
                if (!is_active) {
                    continue;
                }

                auto *rels = atspi_accessible_get_relation_set(tab, nullptr);
                if (!rels) {
                    continue;
                }
                for (size_t rel_ix = 0; rel_ix < rels->len; rel_ix++) {
                    auto *rel = g_array_index(rels, AtspiRelation *, rel_ix); // NOLINT
                    if (!rel) {
                        continue;
                    }
                    if (atspi_relation_get_relation_type(rel) != ATSPI_RELATION_EMBEDS) {
                        continue;
                    }

                    auto *target = atspi_relation_get_target(rel, 0);
                    if (!target) {
                        continue;
                    }
                    to_free.push_back(reinterpret_cast<GObject *>(target));

                    auto *hyperlink = atspi_accessible_get_hyperlink(target);
                    if (hyperlink) {
                        // Chromium
                        to_free.push_back(reinterpret_cast<GObject *>(hyperlink));
                        auto *url_chars = atspi_hyperlink_get_uri(hyperlink, 0, nullptr);
                        if (url_chars) {
                            url = url_chars;
                            g_free(url_chars);
                        }
                    } else {
                        // Firefox
                        auto *doc = atspi_accessible_get_document(target);
                        if (!doc) {
                            continue;
                        }
                        to_free.push_back(reinterpret_cast<GObject *>(doc));

                        std::string prop_name = "DocURL";
                        auto *url_chars =
                            atspi_document_get_attribute_value(doc, prop_name.data(), nullptr);
                        if (url_chars) {
                            url = url_chars;
                            g_free(url_chars);
                        }
                    }
                    break;
                }
                g_array_free(rels, true);
                break;
            }
            break;
        }
    }

    std::for_each(to_free.begin(), to_free.end(), g_object_unref);

    return url;
}

} // namespace keyboard_auto_type
