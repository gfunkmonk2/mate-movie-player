[CCode (cprefix = "Idol", lower_case_cprefix = "idol_")]

namespace Idol {
	[CCode (cheader_filename = "idol.h")]
	public class Object : GLib.Object {
		[CCode (cname = "idol_object_get_type")]
		public static GLib.Type get_type ();

		[CCode (cname = "idol_file_opened")]
		public void file_opened (string mrl);
		[CCode (cname = "idol_file_closed")]
		public void file_closed ();
		[CCode (cname = "idol_metadata_updated")]
		public void metadata_updated (string artist, string title, string album, uint track_num);

		[CCode (cname = "idol_action_error", instance_pos = 3)]
		public void action_error (string title, string reason);

		[CCode (cname = "idol_add_to_playlist_and_play")]
		public void add_to_playlist_and_play (string uri, string display_name, bool add_to_recent);
		[CCode (cname = "idol_action_exit")]
		public void action_exit ();
		[CCode (cname = "idol_action_play")]
		public void action_play ();
		[CCode (cname = "idol_action_stop")]
		public void action_stop ();
		[CCode (cname = "idol_action_play_pause")]
		public void action_play_pause ();
		[CCode (cname = "idol_action_pause")]
		public void action_pause ();
		[CCode (cname = "idol_action_next")]
		public void action_next ();
		[CCode (cname = "idol_action_previous")]
		public void action_previous ();
		[CCode (cname = "idol_action_seek_time")]
		public void action_seek_time (int64 msec, bool accurate);
		[CCode (cname = "idol_action_seek_relative")]
		public void action_seek_relative (int64 offset, bool accurate);

		[CCode (cname = "idol_get_volume")]
		public double get_volume ();
		[CCode (cname = "idol_action_volume")]
		public void action_volume (double volume);
		[CCode (cname = "idol_action_volume_relative")]
		public void action_volume_relative (double off_pct);
		[CCode (cname = "idol_action_volume_toggle_mute")]
		public void action_volume_toggle_mute ();

		[CCode (cname = "idol_action_fullscreen_toggle")]
		public void action_fullscreen_toggle ();
		[CCode (cname = "idol_action_fullscreen")]
		public void action_fullscreen (bool state);

		[CCode (cname = "idol_is_fullscreen")]
		public bool is_fullscreen ();
		[CCode (cname = "idol_is_playing")]
		public bool is_playing ();
		[CCode (cname = "idol_is_paused")]
		public bool is_paused ();
		[CCode (cname = "idol_is_seekable")]
		public bool is_seekable ();

		[CCode (cname = "idol_get_main_window")]
		public Gtk.Window get_main_window ();
		[CCode (cname = "idol_get_ui_manager")]
		public Gtk.UIManager get_ui_manager ();
		[CCode (cname = "idol_get_video_widget")]
		public Gtk.Widget get_video_widget ();

		/* Current media information */
		[CCode (cname = "idol_get_short_title")]
		public string get_short_title ();
		[CCode (cname = "idol_get_current_time")]
		public int64 get_current_time ();

		/* Sidebar handling */
		[CCode (cname = "idol_add_sidebar_page")]
		public void add_sidebar_page (string page_id, string title, Gtk.Widget main_widget);
		[CCode (cname = "idol_remove_sidebar_page")]
		public void remove_sidebar_page (string page_id);
	}

	[CCode (cheader_filename = "idol-cell-renderer-video.h")]
	public class CellRendererVideo : Gtk.CellRenderer {
		[CCode (cname = "idol_cell_renderer_video_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "idol_cell_renderer_video_new")]
		public CellRendererVideo (bool use_placeholder);
	}

	[CCode (cheader_filename = "idol-video-list.h")]
	public class VideoList : Gtk.TreeView {
		[CCode (cname = "idol_video_list_get_type")]
		public static GLib.Type get_type ();
		[CCode (cname = "idol_video_list_new")]
		[CCode (has_construct_function = false)]
		public VideoList ();
		[CCode (cname = "idol_video_list_get_ui_manager")]
		public Gtk.UIManager get_ui_manager ();

		public virtual signal bool starting_video (Gtk.TreePath path);
	}

	[CCode (cheader_filename = "idol-plugin.h")]
	public abstract class Plugin : GLib.Object {
		[CCode (has_construct_function = false)]
		protected Plugin ();

		[CCode (cname = "idol_plugin_get_type")]
		public static GLib.Type get_type ();

		[CCode (cname = "idol_plugin_activate")]
		public abstract bool activate (Idol.Object idol) throws GLib.Error;
		[CCode (cname = "idol_plugin_deactivate")]
		public abstract void deactivate (Idol.Object idol);

		[CCode (cname = "idol_plugin_is_configurable")]
		public virtual bool is_configurable ();
		[CCode (cname = "idol_plugin_create_configure_dialog")]
		public virtual Gtk.Widget create_configure_dialog ();
		[CCode (cname = "idol_plugin_load_interface")]
		public Gtk.Builder load_interface (string name, bool fatal, Gtk.Window parent);

		[CCode (cname = "idol_plugin_find_file")]
		public virtual weak string find_file (string file);
	}
}
