    char eol('\n');
    size_t index = 0;
    EFloat sE, sT, eE, eT;
    utils::ELayoutRetriever retriever(layout);
    std::ofstream out("/home/bwu/code/myRepo/EcadOpt/3rdparty/ecad/examples/script.py");
    out << "import ScriptEnv" << eol;
    out << "ScriptEnv.Initialize(\"Ansoft.ElectronicsDesktop\")" << eol;
    out << "oDesktop.RestoreWindow()" << eol;
    out << "oProject = oDesktop.SetActiveProject(\"CREE62MM\")" << eol;
    out << "oDesign = oProject.SetActiveDesign(\"IcepakDesign1\")" << eol;
    out << "oEditor = oDesign.SetActiveEditor(\"3D Modeler\")" << eol;
    auto primIter = layout->GetPrimitiveIter();
    while (auto * prim = primIter->Next()) {
        if (auto * bw = prim->GetBondwireFromPrimitive(); bw) {
            index++;
            auto start = layout->GetCoordUnits().toUnit(bw->GetStartPt());
            auto end = layout->GetCoordUnits().toUnit(bw->GetEndPt());
            if (bw->GetStartComponent())
                retriever.GetComponentHeightThickness(bw->GetStartComponent(), sE, sT);
            else retriever.GetLayerHeightThickness(bw->GetStartLayer(), sE, sT);
            if (bw->GetEndComponent())
                retriever.GetComponentHeightThickness(bw->GetEndComponent(), eE, eT);
            else retriever.GetLayerHeightThickness(bw->GetEndLayer(), eE, eT);
            auto vec = FPoint3D(end[0], end[1], eE) -  FPoint3D(start[0], start[1], sE);

            out << fmt::Fmt2Str("oEditor.CreateBondwire(") << eol;
            out << fmt::Fmt2Str("\t[") << eol;
		    out << fmt::Fmt2Str("\t\t\"NAME:BondwireParameters\",") << eol;
		    out << fmt::Fmt2Str("\t\t\"WireType:=\"		, \"LOW\",") << eol;
            out << fmt::Fmt2Str("\t\t\"WireDiameter:=\"	, \"%1%mm\",", bw->GetRadius() * 2) << eol;
            out << fmt::Fmt2Str("\t\t\"NumSides:=\"		, \"6\",") << eol;
            out << fmt::Fmt2Str("\t\t\"XPadPos:=\"		, \"%1%mm\",", start[0]) << eol;
            out << fmt::Fmt2Str("\t\t\"YPadPos:=\"		, \"%1%mm\",", start[1]) << eol;
            out << fmt::Fmt2Str("\t\t\"ZPadPos:=\"		, \"%1%mm\",", sE) << eol;
            out << fmt::Fmt2Str("\t\t\"XDir:=\"		, \"%1%mm\",", vec[0]) << eol;
            out << fmt::Fmt2Str("\t\t\"YDir:=\"		, \"%1%mm\",", vec[1]) << eol;
            out << fmt::Fmt2Str("\t\t\"ZDir:=\"		, \"%1%mm\",", vec[2]) << eol;
            out << fmt::Fmt2Str("\t\t\"Distance:=\"		, \"%1%mm\",", vec.Norm2()) << eol;
            out << fmt::Fmt2Str("\t\t\"h1:=\"			, \"%1%mm\",", bw->GetHeight()) << eol;
            out << fmt::Fmt2Str("\t\t\"h2:=\"			, \"0mm\",") << eol;
            out << fmt::Fmt2Str("\t\t\"alpha:=\"		, \"45deg\",") << eol;
            out << fmt::Fmt2Str("\t\t\"beta:=\"		, \"45deg\",") << eol;
            out << fmt::Fmt2Str("\t\t\"WhichAxis:=\"		, \"Z\",") << eol;
            out << fmt::Fmt2Str("\t\t\"ReverseDirection:=\"	, False") << eol;
            out << fmt::Fmt2Str("\t], ") << eol;
            out << fmt::Fmt2Str("\t[") << eol;
            out << fmt::Fmt2Str("\t\t\"NAME:Attributes\",") << eol;
            out << fmt::Fmt2Str("\t\t\"Name:=\"		, \"Bondwire%1%\",", index) << eol;
            out << fmt::Fmt2Str("\t\t\"Flags:=\"		, \"\",") << eol;
            out << fmt::Fmt2Str("\t\t\"Color:=\"		, \"(143 175 143)\",") << eol;
            out << fmt::Fmt2Str("\t\t\"Transparency:=\"	, 0,") << eol;
            out << fmt::Fmt2Str("\t\t\"PartCoordinateSystem:=\", \"Global\",") << eol;
            out << fmt::Fmt2Str("\t\t\"UDMId:=\"		, \"\",") << eol;
            out << fmt::Fmt2Str("\t\t\"MaterialValue:=\"	, \"\\\"Al-Extruded\\\"\",") << eol;
            out << fmt::Fmt2Str("\t\t\"SurfaceMaterialValue:=\", \"\\\"Steel-oxidised-surface\\\"\",") << eol;
            out << fmt::Fmt2Str("\t\t\"SolveInside:=\"		, True,") << eol;
            out << fmt::Fmt2Str("\t\t\"ShellElement:=\"	, False,") << eol;
            out << fmt::Fmt2Str("\t\t\"ShellElementThickness:=\", \"0mm\",") << eol;
            out << fmt::Fmt2Str("\t\t\"ReferenceTemperature:=\", \"20cel\",") << eol;
            out << fmt::Fmt2Str("\t\t\"IsMaterialEditable:=\"	, True,") << eol;
            out << fmt::Fmt2Str("\t\t\"UseMaterialAppearance:=\", False,") << eol;
            out << fmt::Fmt2Str("\t\t\"IsLightweight:=\"	, False") << eol;
            out << fmt::Fmt2Str("\t])") << eol;
            // break;
        }
    }
    out.close();