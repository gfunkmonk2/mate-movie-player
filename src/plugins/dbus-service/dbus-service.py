## Idol D-Bus plugin
## Copyright (C) 2009 Lucky <lucky1.data@gmail.com>
## Copyright (C) 2009 Philip Withnall <philip@tecnocode.co.uk>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor,
## Boston, MA 02110-1301  USA.
##
## Sunday 13th May 2007: Bastien Nocera: Add exception clause.
## See license_change file for details.

import idol
import gobject, gtk
import dbus, dbus.service
from dbus.mainloop.glib import DBusGMainLoop

class dbusservice(idol.Plugin):
	def __init__(self):
		idol.Plugin.__init__(self)

	def activate(self, idol):
		DBusGMainLoop(set_as_default = True)

		name = dbus.service.BusName ('org.mpris.Idol', bus = dbus.SessionBus ())
		self.root = Root (name, idol)
		self.player = Player (name, idol)
		self.track_list = TrackList (name, idol)

	def deactivate(self, idol):
		self.root.disconnect() # ensure we don't leak our paths on the bus
		self.player.disconnect()
		self.track_list.disconnect()

class Root (dbus.service.Object):
	def __init__(self, name, idol):
		dbus.service.Object.__init__ (self, name, '/')
		self.idol = idol

	def disconnect(self):
		self.remove_from_connection(None, None)

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='s')
	def Identity(self):
		return self.idol.get_version()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='')
	def Quit(self):
		self.idol.action_exit()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='(qq)')
	def MprisVersion(self):
		return dbus.Struct((dbus.UInt16(1), dbus.UInt16(0)), signature='(qq)')

class Player(dbus.service.Object):
	def __init__(self, name, idol):
		dbus.service.Object.__init__(self, name, '/Player')
		self.idol = idol

		self.null_metadata = {"year" : "", "tracknumber" : "", "location" : "",
			"title" : "", "album" : "", "time" : "", "genre" : "", "artist" : ""}
		self.old_metadata = self.null_metadata.copy()
		self.current_metadata = self.null_metadata.copy()
		self.old_caps = 64 # at startup, we can only support a playlist
		self.old_status = (2, 0, 0, 0) # startup state

		idol.connect("metadata-updated", self.do_update_metadata)
		idol.connect("notify::playing", self.do_notify)
		idol.connect("notify::seekable", self.do_notify)
		idol.connect("notify::current-mrl", self.do_notify)

	def do_update_metadata(self, idol, artist, title, album, num):
		self.current_metadata = self.null_metadata.copy()
		if title:
			self.current_metadata["title"] = title
		if artist:
			self.current_metadata["artist"] = artist
		if album:
			self.current_metadata["album"] = album
		if num:
			self.current_metadata["tracknumber"] = num

		if idol.is_playing():
			self.track_change(self.current_metadata)

	def do_notify(self, idol, status):
		if idol.is_playing():
			self.track_change(self.current_metadata)
		else:
			self.track_change(self.null_metadata)

		status = self.calculate_status()
		if status != self.old_status:
			self.status_change(status)

		caps = self.calculate_caps()
		if caps != self.old_caps:
			self.caps_change(caps)

	def calculate_status(self):
		if self.idol.is_playing():
			playing_status = 0
		elif self.idol.is_paused():
			playing_status = 1
		else:
			playing_status = 2

		if self.idol.action_remote_get_setting(idol.REMOTE_SETTING_SHUFFLE):
			shuffle_status = 1
		else:
			shuffle_status = 0

		if self.idol.action_remote_get_setting(idol.REMOTE_SETTING_REPEAT):
			repeat_status = 1
		else:
			repeat_status = 0

		return (
			dbus.Int32(playing_status), # 0 = Playing, 1 = Paused, 2 = Stopped
			dbus.Int32(self.idol.action_remote_get_setting(idol.REMOTE_SETTING_SHUFFLE)), # 0 = Playing linearly , 1 = Playing randomly
			dbus.Int32(0), # 0 = Go to the next element once the current has finished playing , 1 = Repeat the current element 
			dbus.Int32(self.idol.action_remote_get_setting(idol.REMOTE_SETTING_REPEAT)) # 0 = Stop playing once the last element has been played, 1 = Never give up playing 
		)

	def calculate_caps(self):
		caps = 64 # we can always have a playlist
		playlist_length = self.idol.get_playlist_length()
		playlist_pos = self.idol.get_playlist_pos()

		if playlist_pos < playlist_length - 1:
			caps |= 1 << 0 # go next
		if playlist_pos > 0:
			caps |= 1 << 1 # go previous
		if playlist_length > 0:
			caps |= 1 << 2 # pause
			caps |= 1 << 3 # play
		if self.idol.is_seekable():
			caps |= 1 << 4 # seek
		if self.current_metadata != self.null_metadata:
			caps |= 1 << 5 # get metadata

		return caps

	def track_change(self, metadata):
		if self.old_metadata != metadata:
			self.old_metadata = metadata.copy()
			self.TrackChange(metadata)

	def status_change(self, status):
		if self.old_status != status:
			self.old_status = status
			self.StatusChange(status)

	def caps_change(self, caps):
		if self.old_caps != caps:
			self.old_caps = caps
			self.CapsChange(caps)

	def disconnect(self):
		self.TrackChange(self.null_metadata)
		self.remove_from_connection(None, None)

	@dbus.service.signal(dbus_interface = "org.freedesktop.MediaPlayer", signature='a{sv}')
	def TrackChange(self, metadata):
		pass

	@dbus.service.signal(dbus_interface = "org.freedesktop.MediaPlayer", signature='(iiii)')
	def StatusChange(self, status):
		pass

	@dbus.service.signal(dbus_interface = "org.freedesktop.MediaPlayer", signature='i')
	def CapsChange(self, caps):
		pass

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='')
	def Next(self):
		self.idol.action_next()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='')
	def Prev(self):
		self.idol.action_previous()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='')
	def Pause(self):
		self.idol.action_play_pause()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='')
	def Stop(self):
		self.idol.action_stop()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='')
	def Play(self):
		# If playing : rewind to the beginning of current track, else : start playing. 
		if self.idol.is_playing():
			self.idol.action_seek_time(0)
		else:
			self.idol.action_play()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='b', out_signature='')
	def Repeat(self, value):
		pass # we don't support repeating individual tracks

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='(iiii)')
	def GetStatus(self):
		status = self.calculate_status()
		self.old_status = status
		return dbus.Struct(status, signature='(iiii)')

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='a{sv}')
	def GetMetadata(self):
		return self.current_metadata

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='i')
	def GetCaps(self):
		caps = self.calculate_caps()
		self.old_caps = caps
		return caps

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='i', out_signature='')
	def VolumeSet(self, volume):
		self.idol.action_volume(volume / 100.0)

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='i')
	def VolumeGet(self):
		return dbus.Int32(self.idol.get_volume() * 100)

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='i', out_signature='')
	def PositionSet(self, position):
		self.idol.action_seek_time(position)

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='i')
	def PositionGet(self):
		return dbus.Int32(self.idol.props.current_time)

class TrackList(dbus.service.Object):
	def __init__(self, name, idol):
		dbus.service.Object.__init__(self, name, '/TrackList')
		self.idol = idol

	def disconnect(self):
		self.remove_from_connection(None, None)

	@dbus.service.signal(dbus_interface = "org.freedesktop.MediaPlayer", signature='i')
	def TrackListChange(self, length):
		# TODO: we can't implement this until IdolPlaylist is exposed in the Python API
		pass

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='i', out_signature='a{sv}')
	def GetMetadata(self, pos):
		# Since the API doesn't currently exist in Idol to get the rest of the metadata, we can only return the title
		return { "title" : self.idol.get_title_at_playlist_pos(pos) }

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='i')
	def GetCurrentTrack(self):
		return self.idol.get_playlist_pos()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='', out_signature='i')
	def GetLength(self):
		return self.idol.get_playlist_length()

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='sb', out_signature='i')
	def AddTrack(self, uri, play_immediately):
		# We can't currently support !play_immediately
		self.idol.add_to_playlist_and_play(uri, '', True)
		return 0

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='i', out_signature='')
	def DelTrack(self, pos):
		# TODO: we need IdolPlaylist exposed by the Python API for this
		pass

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='b', out_signature='')
	def SetLoop(self, loop):
		self.idol.action_remote_set_setting(idol.REMOTE_SETTING_REPEAT, loop)

	@dbus.service.method(dbus_interface='org.freedesktop.MediaPlayer', in_signature='b', out_signature='')
	def SetRandom(self, random):
		self.idol.action_remote_set_setting(idol.REMOTE_SETTING_SHUFFLE, random)
