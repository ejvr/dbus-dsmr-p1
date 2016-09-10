import qbs.base 1.0

Product {
	Depends { name: "cpp" }
	Depends { name: "Qt"; submodules: ["core", "dbus", "serialport", "xml"] }
	VeQItems {}
	VeQItemsDbus {}
	name: "dbus-p1meter"
	type: "application"
	cpp.cxxLanguageVersion: "c++11"
	cpp.defines: ["VERSION=\"0.1.0\""]
	cpp.includePaths: [
		"ext/velib/inc",
		"ext/velib/inc/velib/platform",
		"src"
	]
	cpp.cppFlags: [
		// suppress the mangling of va_arg has changed for gcc 4.4
		"-Wno-psabi",
		// these warnings appear when compiling with QT4.8.3-debug. Problem appears to be solved in
		// newer QT versions.
		"-Wno-unused-local-typedefs"
	]
	consoleApplication: true
	files: [
		"src/main.cpp",
		"src/crc.h",
		"src/dsmr_p1_acquisitor.cpp",
		"src/dsmr_p1_acquisitor.h",
		"src/dsmr_p1_parser.cpp",
		"src/dsmr_p1_parser.h",
		"src/velib/velib_config_app.h",
		"../README.md"
	]
}
