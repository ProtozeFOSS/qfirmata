import qbs
import qbs.File
DynamicLibrary{
    name:"qfirmata"
    id:qFirmata
    property string libName:(qbs.targetOS.contains("windows") ? "":"lib") + name + (qbs.targetOS.contains("windows") ? ".dll":".so")
    Depends{name:'cpp'}
    cpp.cxxLanguageVersion: "c++11"
    Depends{name:  "Qt"; submodules:["core", "serialport", "websockets"]}
    Export{
        Depends{name:'cpp'}
        cpp.cxxLanguageVersion: "c++11"
        Depends{name:  "Qt"; submodules:["core", "serialport","websockets"]}
        cpp.includePaths:[qbs.installRoot + "/include/" + qFirmata.name];
        cpp.libraryPaths:[qbs.installRoot + "/lib" ];
        cpp.dynamicLibraries:[qbs.installRoot + "/lib/" + qFirmata.libName ]
    }
    cpp.defines:['QFIRMATA_LIBRARY']
    Group {
        qbs.install: true
        fileTagsFilter: "dynamiclibrary"
        qbs.installDir:"/lib"
    }
    Group{
         name:"headers"
         files: [
            "src/qfirmataconnection.h",
            "src/qfirmataparser.h",
        ]
        qbs.install: true
        qbs.installDir:"/include/qfirmata"
    }
    Group{
        name:"private"
        files: [
            "src/private/qfirmata_p.h",
        ]
        qbs.install: true
        qbs.installDir:"/include/qfirmata/private"
    }

    Group{
        name:"source"
        files:[
            "src/qfirmataconnection.cpp",
            "src/qfirmataparser.cpp",
        ]
        qbs.install:false
    }
}
