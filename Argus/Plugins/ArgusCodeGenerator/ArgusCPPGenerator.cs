using EpicGames.Core;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ArgusCodeGenerator
{
    internal class ArgusCPPGenerator : ScriptCodeGeneratorBase
    {
        public ArgusCPPGenerator(IUhtExportFactory factory) : base(factory)
        {
        }

        protected override void ExportClass(StringBuilder builder, UhtClass classObj)
        {
            throw new NotImplementedException();
        }

        protected override void Finish(StringBuilder builder, List<UhtClass> classes)
        {
            throw new NotImplementedException();
        }
    }
}
