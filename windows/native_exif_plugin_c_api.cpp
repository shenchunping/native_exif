#include "include/native_exif/native_exif_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "native_exif_plugin.h"

void NativeExifPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  native_exif::NativeExifPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
