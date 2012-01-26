# From code by James Livingston

import idol
import gobject, gtk

class SamplePython(idol.Plugin):

	def __init__(self):
		idol.Plugin.__init__(self)
			
	def activate(self, idol):
		print "Activating sample Python plugin"
		idol.action_fullscreen_toggle()
	
	def deactivate(self, idol):
		print "Deactivating sample Python plugin"
