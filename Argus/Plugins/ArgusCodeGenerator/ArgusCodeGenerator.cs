using EpicGames.UHT.Tables;
using EpicGames.UHT.Utils;

namespace ArgusCodeGenerator
{
    [UnrealHeaderTool]
    public class ArgusCodeGenerator
    {
        [UhtExporter(Name = "ArgusCodeGenerator", Description = "Code generation for the Argus project", Options = UhtExporterOptions.Default, ModuleName = "ArgusCodeGenerator")]
        private static void ScriptGeneratorExporter(IUhtExportFactory Factory)
        {
            // Make sure this plugin should be run
            if (!Factory.Session.IsPluginEnabled("ArgusCodeGenerator", false))
            {
                return;
            }

            // Based on the WITH_LUA setting, run the proper exporter.
            if (Factory.PluginModule != null)
            {
                new ArgusCPPGenerator(Factory).Generate();
            }
        }
    }
}
