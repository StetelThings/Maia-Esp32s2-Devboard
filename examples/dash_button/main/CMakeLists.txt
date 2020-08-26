# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)
idf_component_register(SRCS "dash_button_example.c"
                    INCLUDE_DIRS ".")
#                    EMBED_TXTFILES howsmyssl_com_root_cert.pem)
