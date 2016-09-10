import qbs

Group {
	name: "VeQItemsDbus"
	files: [
		"ext/velib/src/qt/v_busitem_proxy.h",
		"ext/velib/src/qt/ve_qitem_dbus_virtual_object.hpp",
		"ext/velib/inc/velib/qt/ve_qitems_dbus.hpp",
		"ext/velib/inc/velib/qt/ve_qitem_dbus_publisher.hpp",
		"ext/velib/src/qt/v_busitem_proxy.cpp",
		"ext/velib/src/qt/ve_qitems_dbus.cpp",
		"ext/velib/src/qt/ve_qitem_dbus_virtual_object.cpp",
		"ext/velib/src/qt/ve_qitem_dbus_publisher.cpp",
	]
}
