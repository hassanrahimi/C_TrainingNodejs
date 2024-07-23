#include <gst/gst.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

// Define the function to be exported
#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

typedef struct _Participant
{
    GstElement *source;
    GstElement *jitterbuffer;
    GstElement *depayloader;
    GstElement *audioconvert;
    GstElement *mixer;
    GstElement *encoder;
    GstElement *rtppay;
    GstElement *sink;
    GstPad *sinkpad;
    GstPad *srcpad;
    gchar *ip;
    gint port;
} Participant;

GstElement *pipeline ;
GHashTable *participants ;
GMainLoop *loop ;
GstBus *bus ;
guint bus_watch_id ;
// GstElement *pipeline = NULL;
// GHashTable *participants = NULL;
// GMainLoop *loop = NULL;
// GstBus *bus = NULL;
// guint bus_watch_id = 0;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;
    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_EOS:
        g_print("End of stream\n");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_ERROR:
    {

        gchar *debug;
        GError *error;
        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);
        g_printerr("Error: %s\n", error->message);
        g_error_free(error);

        g_main_loop_quit(loop);
        break;
    }
    default:
        break;
    }

    return TRUE;
}
DLL_EXPORT int createPipline(gchar *piplineName)
{
    gst_init(NULL, NULL);
    pipeline = gst_pipeline_new(piplineName);

    if (!pipeline)
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    loop = g_main_loop_new(NULL, FALSE);
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    // parameters : (GstBus *bus, GstBusFunc func, gpointer user_data)
    bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    // start playing the pipeline
    g_print("Now playing \n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // initialize participant hash table
    participants = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    //
    // Run the loop
    // g_main_loop_run(loop);
    // Free resources
    // g_print("Returned, stopping playback\n");
    // gst_element_set_state(pipeline, GST_STATE_NULL);

    // g_print("Deleting pipeline\n");
    // gst_object_unref(GST_OBJECT(pipeline));
    // g_source_remove(bus_watch_id);
    // g_main_loop_unref(loop);

    return 1;
}
// این یک تابع می باشد که به عنوان تابع بازگشتی مورد استفاده قرار گرفته است
static void pad_added_handler(GstElement *src, GstPad *pad, gpointer data)
{
    printf("pad_added_handler");
    Participant *participant = (Participant *)data;
    GstPad *sinkpad = participant->sinkpad;

    // شاید باید این را هم اضافه کنم در یک مثال دیگه که درست شد این بود توسط هوش مصنوعی
    //  sinkpad = gst_element_get_static_pad(participant->jitterbuffer, "sink");
    // gst_pad_link(pad, sinkpad);
    // و همینطور این
    //   gst_object_unref(sinkpad);

      if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link dynamic pad.\n");
    }
}

DLL_EXPORT void add_participant(gchar *protocol, gchar *ip, gchar *port, gchar *media_ip, gchar *media_port)
{
    if(pipeline){
        printf("pipeline exists");

    }else{
        printf("pipline not exist");
    }
    printf("Received parameters: protocol=%s IP=%s, Port=%s, Media IP=%s, Media Port=%s\n", protocol, ip, port, media_ip, media_port);
    gboolean link_status;
    Participant *participant = g_malloc(sizeof(Participant));
    gchar *name = g_strdup_printf("participant_%s_%d", ip, atoi(port));
    printf("adding participant to pipeline");
    participant->ip = ip;
    participant->port = atoi(port);
    // نامی که در خط بالا ایجاد شده استفاده میشود برای نام گذاری داده های دریافتی آن کاربر و دیگر نیازمندی های آن
    // participant->source = gst_element_factory_make("udpsrc", name);
    if (g_strcmp0(protocol, "udp") == 0)
    {
        participant->source = gst_element_factory_make("udpsrc", name);
        g_object_set(participant->source, "port", atoi(port), NULL);
    }
    else if (g_strcmp0(protocol, "tcp") == 0)
    {
        participant->source = gst_element_factory_make("tcpclientsrc", name);
        g_object_set(participant->source, "host", ip, "port", atoi(port), NULL);
    }
    else
    {
        g_printerr("Unsupported protocol: %s\n", protocol);
        g_free(participant);
        return;
    }
    participant->jitterbuffer = gst_element_factory_make("rtpjitterbuffer", name);
    participant->depayloader = gst_element_factory_make("rtpL16depay", name);
    participant->audioconvert = gst_element_factory_make("audioconvert", name);

    participant->mixer = gst_element_factory_make("audiomixer", name);
    participant->encoder = gst_element_factory_make("opusenc", name);
    participant->rtppay = gst_element_factory_make("rtpopuspay", name);
    participant->sink = gst_element_factory_make("udpsink", name);
    printf("adding participant to pipeline 1\n");
    if (!participant->source || !participant->jitterbuffer || !participant->depayloader || !participant->audioconvert ||
        !participant->mixer || !participant->encoder || !participant->rtppay || !participant->sink)
    {
        g_printerr("Failed to create elements for participant %s:%d\n", ip, atoi(port));
        return;
    }
    printf("adding participant to pipeline 2\n");
    // set properties of source
    g_object_set(participant->source, "port", atoi(port), NULL);
    if (g_strcmp0(protocol, "udp") == 0)
    {
        g_object_set(participant->source, "caps", gst_caps_from_string("application/x-rtp,media=audio,clock-rate=48000,payload=96"), NULL);
    }

    // Ip and port related plainTransport مربوط به این می باشد خیلی مهم
    g_object_set(participant->sink, "host", media_ip, "port", atoi(media_port), NULL);
    printf("adding participant to pipeline 2 1\n");
    // pipleline chain ایجاد زنجیر پایپلاین
    gst_bin_add_many(GST_BIN(pipeline), participant->source, participant->jitterbuffer, participant->depayloader,
                     participant->audioconvert, participant->mixer, participant->encoder, participant->rtppay, participant->sink, NULL);
    printf("adding participant to pipeline 2 1\n");
    link_status = gst_element_link_many(participant->source, participant->jitterbuffer, participant->depayloader, participant->audioconvert, participant->mixer, NULL);
    if (!link_status)
    {
        g_printerr("Failed to link elements for participant %s:%d in the first chain\n", ip, atoi(port));
        return;
    }
    printf("adding participant to pipeline 2 1\n");
    link_status = gst_element_link_many(participant->mixer, participant->encoder, participant->rtppay, participant->sink, NULL);
    if (!link_status)
    {
        g_printerr("Failed to link elements for participant %s:%d in the second chain\n", ip, atoi(port));
        return;
    }
    printf("adding participant to pipeline 3\n");
    // مثل اینکه تاریخ مصرفش گذشته بود که کامپایلر زبان سی ایراد گرفت
    //  GstPad *mixer_sink_pad = gst_element_get_request_pad(participant->mixer, "sink_%u");
    GstPad *mixer_sink_pad = gst_element_request_pad_simple(participant->mixer, "sink_%u");
    participant->sinkpad = mixer_sink_pad;
    GstPad *audio_convert_src_pad = gst_element_get_static_pad(participant->audioconvert, "src");
    printf("adding participant to pipeline 4\n");
    gst_pad_link(audio_convert_src_pad, mixer_sink_pad);
    gst_object_unref(audio_convert_src_pad);
    // تابعی که در بالا ایجاد شده به عنوان تابع بازگشتی در این قسمت قرار داده شده است
    g_signal_connect(participant->source, "pad-added", G_CALLBACK(pad_added_handler), participant);
    printf("adding participant to pipeline 4\n");
    // یک کلید از ای پی و پورت ایجاد میکنیم برای کلید ذخیره کاربر
    gchar *key = g_strdup_printf("%s:%d", ip, atoi(port));
    // در جدول مخاطبین با کلید مشخص یک مخاطب اضافه میشود
    // پارامتر اول : جدول مخاطبین است
    // پارامتر دوم : کلید می باشد
    //  پارامتر سوم : مقدار می باشد که یک مخاطب ایجاد شده در مراحل قبل می باشد
    g_hash_table_insert(participants, key, participant);
    printf("adding participant to pipeline 5\n");
    // تغییر دادن وضعیت ها برای اجرا شدن
    gst_element_set_state(participant->source, GST_STATE_PLAYING);
    gst_element_set_state(participant->jitterbuffer, GST_STATE_PLAYING);
    gst_element_set_state(participant->depayloader, GST_STATE_PLAYING);
    gst_element_set_state(participant->audioconvert, GST_STATE_PLAYING);
    gst_element_set_state(participant->mixer, GST_STATE_PLAYING);
    gst_element_set_state(participant->encoder, GST_STATE_PLAYING);
    gst_element_set_state(participant->rtppay, GST_STATE_PLAYING);
    gst_element_set_state(participant->sink, GST_STATE_PLAYING);
}

void remove_participant(gchar *ip, gint port)
{
    //
    gchar *key = g_strdup_printf("%s:%d", ip, port);

    // از جدول موجود با کلید مورد نظر کاربر را پیدا می کنیم
    Participant *participant = g_hash_table_lookup(participants, key);

    if (!participant)
    {
        g_printerr("No participant found for %s:%d\n", ip, port);
        g_free(key);
        return;
    }

    gst_element_set_state(participant->source, GST_STATE_NULL);
    gst_element_set_state(participant->jitterbuffer, GST_STATE_NULL);
    gst_element_set_state(participant->depayloader, GST_STATE_NULL);
    gst_element_set_state(participant->audioconvert, GST_STATE_NULL);
    gst_element_set_state(participant->mixer, GST_STATE_NULL);
    gst_element_set_state(participant->encoder, GST_STATE_NULL);
    gst_element_set_state(participant->rtppay, GST_STATE_NULL);
    gst_element_set_state(participant->sink, GST_STATE_NULL);

    // remove from pipeline
    gst_bin_remove_many(GST_BIN(pipeline), participant->source, participant->jitterbuffer, participant->depayloader,
                        participant->audioconvert, participant->mixer, participant->encoder, participant->rtppay, participant->sink, NULL);

    gst_element_release_request_pad(participant->mixer, participant->sinkpad);
    gst_object_unref(participant->sinkpad);

    // کاربر از جدول با کلید ایجاد شده حذف میشود
    g_hash_table_remove(participants, key);
    g_free(participant->ip);
    g_free(participant);
}

// int main (int argc, char **argv[]){
//     printf("main function ++++++");
// }
// int main(int argc, char *argv[])
// {

//     // آی پی و پورت های مربوط به جریان ایجاد شده و پلین ترنفسفر دریافت کننده را دریافت می کنیم و قرار می دهیم
//     gchar *functionName = argv[1];
//     gchar *ip = argv[2];
//     gint port = atoi(argv[3]);
//     gchar *roomId = argv[4];
//     gchar *userId = argv[5];
//     gchar *media_ip = argv[6];
//     gint media_port = atoi(argv[7]);

//     GMainLoop *loop;
//     GstBus *bus;
//     guint bus_watch_id;

//     // initialize Gstreamer
//     gst_init(&argc, &argv);

//     // create the element
//     // پارامتر اول نام تابع می باشد که به وسیله آن فراخوانی میشود و باید دقیق مطابق باشد
//     // پارامتر دوم نامی است که برای این میکسر انتخاب میشود که میتواند متغییر باشد
//     // به داخل استراکچر منتقل شد
//     // mixer = gst_element_factory_make("audiomixer", "mixer");
//     // encoder = gst_element_factory_make("opusenc", "encoder");
//     // rtppay = gst_element_factory_make("rtpopuspay", "rtppay");

//     // create the empty pipeline

//     if (!pipeline)
//     {
//         printf("create pipeline");
//         pipeline = gst_pipeline_new("audio-mixer-pipeline");
//         if (!pipeline)
//         {
//             g_printerr("Not all elements could be created.\n");
//             return -1;
//         }
//     }
//     // به داخل استراکچر منتقل شد
//     //  gst_bin_add_many(GST_BIN(pipeline), mixer, encoder, rtppay, NULL);

//     // if (gst_element_link_many(mixer, encoder, rtppay, NULL) != TRUE)
//     // {
//     //     g_printerr("Mixer, encoder, and payloader could not be linked.\n");
//     //     gst_object_unref(pipeline);
//     //     return -1;
//     // }

//     // create a GLib Main Loop and set bus watch
//     // parameters : (GMainContext *context, gboolean is_running)
//     loop = g_main_loop_new(NULL, FALSE);
//     bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
//     // parameters : (GstBus *bus, GstBusFunc func, gpointer user_data)
//     bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
//     gst_object_unref(bus);

//     // start playing the pipeline
//     g_print("Now playing \n");
//     gst_element_set_state(pipeline, GST_STATE_PLAYING);

//     // initialize participant hash table
//     participants = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

//     if (strcmp(functionName, "addParticipant") == 0)
//     {
//         add_participant(ip, port, media_ip, media_port);
//     }
//     else if (strcmp(functionName, "removeParticipant") == 0)
//     {
//         remove_participant(ip, port);
//     }
//     //
//     // Run the loop
//     g_main_loop_run(loop);
//     // Free resources
//     g_print("Returned, stopping playback\n");
//     gst_element_set_state(pipeline, GST_STATE_NULL);

//     g_print("Deleting pipeline\n");
//     gst_object_unref(GST_OBJECT(pipeline));
//     g_source_remove(bus_watch_id);
//     g_main_loop_unref(loop);
//     return 0;
// }