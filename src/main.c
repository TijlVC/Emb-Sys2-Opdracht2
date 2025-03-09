#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PJ_RPI_USER i.p.v. PJ_RPI
#include "PJ_RPI_USER.h"

// Paho MQTT
#include "MQTTClient.h"

// ---------------------------------------------------
// ===========   MQTT SETTINGS  ======================
#define MQTT_ADDRESS   "tcp://localhost:1883"
#define MQTT_CLIENTID  "RpiSubscriber"
#define MQTT_TOPIC     "tc74/temperature"
#define MQTT_QOS       1

// ---------------------------------------------------
// ===========   GLOBALS  ============================
static MQTTClient client;

// GUI widgets
static GtkWidget *label_temp;     // Toon de temperatuur
static GtkWidget *entry_outpin;   // Hierin vult user bcm pin in die output is
static GtkWidget *entry_inpin;    // Hierin vult user bcm pin in die input is
static GtkWidget *label_io;       // Om inputpin‐state te tonen

static int is_gpio_mapped = 0;    // 0=niet gemapped, 1=wel gemapped

// ---------------------------------------------------
// ===========   PJ_RPI_USER HELPER FUNCTIES  =============

// map gpiomem (via PJ_RPI_USER), zodat we GPIO_SET, GPIO_CLR enz. kunnen doen
static int init_gpiomem(void)
{
    if (map_peripheral(&gpio) == -1) {
        fprintf(stderr, "Failed to open /dev/gpiomem, try checking permissions.\n");
        return -1;
    }
    is_gpio_mapped = 1;
    return 0;
}

// Stel pin p in als output (BCM numbering)
static void set_pin_output(int p)
{
    INP_GPIO(p);
    OUT_GPIO(p);
}

// Stel pin p in als input
static void set_pin_input(int p)
{
    INP_GPIO(p);
}

// Zet pin p hoog
static void set_pin_high(int p)
{
    GPIO_SET = 1 << p;
}

// Zet pin p laag
static void set_pin_low(int p)
{
    GPIO_CLR = 1 << p;
}

// Lees pin p (0 of 1)
static int read_pin(int p)
{
    unsigned int levelReg = *(gpio.addr + 13);
    int val = (levelReg & (1 << p)) ? 1 : 0;
    return val;
}

// ---------------------------------------------------
// ===========   MQTT CALLBACKS  =====================
static int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    // Payload is bv "21"
    char *payload = (char*)message->payload;

    // label_temp updaten
    char buf[64];
    snprintf(buf, sizeof(buf), "Temperature: %s °C", payload);
    gtk_label_set_text(GTK_LABEL(label_temp), buf);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// ---------------------------------------------------
// ===========   GTK CALLBACKS  ======================

static void on_button_toggle_clicked(GtkWidget *widget, gpointer user_data)
{
    // Lees pin nrs uit
    const char *outpin_str = gtk_entry_get_text(GTK_ENTRY(entry_outpin));
    const char *inpin_str  = gtk_entry_get_text(GTK_ENTRY(entry_inpin));

    int outpin = atoi(outpin_str);
    int inpin  = atoi(inpin_str);

    // map gpiomem als dat nog niet gedaan is
    if(!is_gpio_mapped) {
        if(init_gpiomem() < 0) {
            return; // mislukt => geen toggling
        }
    }

    // output pin config
    set_pin_output(outpin);

    // toggle (lees de huidige waarde, flip 'm)
    int curVal = read_pin(outpin);
    if(curVal == 1) {
        set_pin_low(outpin);
    } else {
        set_pin_high(outpin);
    }

    // input pin config
    set_pin_input(inpin);

    // input uitlezen
    int inVal = read_pin(inpin);

    // label_io updaten
    char buff[64];
    snprintf(buff, sizeof(buff), "Input pin state: %d", inVal);
    gtk_label_set_text(GTK_LABEL(label_io), buff);
}

// ---------------------------------------------------
// ===========   MAIN  ===============================
int main(int argc, char *argv[])
{
    // 1) MQTT client (subscriber)
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_create(&client, MQTT_ADDRESS, MQTT_CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    // We zetten een callback om messages te ontvangen
    MQTTClient_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    if(MQTTClient_connect(client, &conn_opts) == MQTTCLIENT_SUCCESS) {
        MQTTClient_subscribe(client, MQTT_TOPIC, MQTT_QOS);
    } else {
        fprintf(stderr, "Failed to connect to MQTT broker (topic won't update)\n");
    }

    // 2) GTK init
    gtk_init(&argc, &argv);

    // venster
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK + MQTT + PJ_RPI_USER Demo");
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // layout vbox
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // label temperature
    label_temp = gtk_label_new("Temperature: (waiting)");
    gtk_box_pack_start(GTK_BOX(vbox), label_temp, FALSE, FALSE, 0);

    // Hbox: Output pin
    GtkWidget *hbox_out = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_out, FALSE, FALSE, 0);

    GtkWidget *lbl_out = gtk_label_new("Output pin (BCM): ");
    gtk_box_pack_start(GTK_BOX(hbox_out), lbl_out, FALSE, FALSE, 0);

    entry_outpin = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_outpin), "17");
    gtk_box_pack_start(GTK_BOX(hbox_out), entry_outpin, FALSE, FALSE, 0);

    // Hbox: Input pin
    GtkWidget *hbox_in = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_in, FALSE, FALSE, 0);

    GtkWidget *lbl_in = gtk_label_new("Input pin (BCM): ");
    gtk_box_pack_start(GTK_BOX(hbox_in), lbl_in, FALSE, FALSE, 0);

    entry_inpin = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_inpin), "27");
    gtk_box_pack_start(GTK_BOX(hbox_in), entry_inpin, FALSE, FALSE, 0);

    // Label voor input
    label_io = gtk_label_new("Input pin state: ?");
    gtk_box_pack_start(GTK_BOX(vbox), label_io, FALSE, FALSE, 0);

    // Knop "Toggle Output Pin"
    GtkWidget *btn_toggle = gtk_button_new_with_label("Toggle Output Pin");
    g_signal_connect(btn_toggle, "clicked", G_CALLBACK(on_button_toggle_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn_toggle, FALSE, FALSE, 0);

    // show all
    gtk_widget_show_all(window);

    // 3) GTK main loop
    gtk_main();

    // 4) Clean up
    MQTTClient_unsubscribe(client, MQTT_TOPIC);
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    if(is_gpio_mapped) {
        unmap_peripheral(&gpio);
    }

    return 0;
}
