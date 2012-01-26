using GLib;
using Idol;

class SampleValaPlugin: Idol.Plugin {
	public override bool activate (Idol.Object idol) throws GLib.Error {
		stdout.printf ("Hello world\n");
		return true;
	}

	public override void deactivate (Idol.Object idol) {
		stdout.printf ("Goodbye world\n");
	}
}


[ModuleInit]
public GLib.Type register_idol_plugin (GLib.TypeModule module)
{
	stdout.printf ("Registering plugin %s\n", "SampleValaPlugin");

	return typeof (SampleValaPlugin);
}
