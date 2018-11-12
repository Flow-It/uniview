#include <poppler.h>
#include <gtk/gtk.h>


struct render_context {
	PopplerDocument *doc;
	PopplerPage *page;
	int num_pages;
	int cur_page;
};

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    struct render_context *ctx = user_data;

    fprintf(stderr, "key press event start, cur_page = %d\n", ctx->cur_page);
    switch (event->keyval) {
	    case GDK_KEY_Left:
	    case GDK_KEY_Down:
		if (ctx->cur_page > 0) {
			ctx->cur_page--;
			ctx->page = poppler_document_get_page(ctx->doc, ctx->cur_page);
		}
		break;
	    case GDK_KEY_Right:
	    case GDK_KEY_Up:
		if (ctx->cur_page < ctx->num_pages - 1) {
			ctx->cur_page++;
			ctx->page = poppler_document_get_page(ctx->doc, ctx->cur_page);
		}
		break;
	    default:
		break;
    }

    fprintf(stderr, "key press event end, cur_page = %d\n", ctx->cur_page);
    gtk_widget_queue_draw(widget);
    return FALSE;
}

gboolean page_draw(GtkWidget *widget, cairo_t *cr, gpointer userdata)
{
	struct render_context *ctx = userdata;
	fprintf(stderr, "drawing page %d\n", ctx->cur_page);
	poppler_page_render(ctx->page, cr);
	return FALSE;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;

    gtk_init(&argc, &argv);

    struct render_context ctx;

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect_swapped(G_OBJECT(win), "destroy",
                             G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *widget = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(win), widget);

    char buf[1024];
    sprintf(buf, "file:%s", argv[1]);
    GError *err = NULL;
    ctx.doc = poppler_document_new_from_file(buf, NULL, &err);
    if (!ctx.doc) {
	printf("Error opening document: %s\n", err->message);
        return 2;
    }
    ctx.page = poppler_document_get_page(ctx.doc, 0);
    ctx.cur_page = 0;
    ctx.num_pages = poppler_document_get_n_pages(ctx.doc);


    g_signal_connect(widget, "draw", G_CALLBACK(page_draw), &ctx);
    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key_press), &ctx);
    gtk_widget_show_all(win);

    gtk_main();

    return 0;
}
