#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <memory>
#include <string>
#include <map>

class NativeExifPlugin : public flutter::Plugin {
  private:
      std::unique_ptr<NativeExifWindows> exifHandler;
      std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;
  
  public:
      static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) {
          auto channel = std::make_unique<flutter::MethodChannel<>>(
              registrar->messenger(),
              "native_exif",
              &flutter::StandardMethodCodec::GetInstance()
          );
  
          auto* plugin = new NativeExifPlugin();
          channel->SetMethodCallHandler(
              [plugin_pointer = plugin](const auto& call, auto result) {
                  plugin_pointer->HandleMethodCall(call, std::move(result));
              }
          );
  
          plugin->channel = std::move(channel);
          plugin->exifHandler = std::make_unique<NativeExifWindows>();
      }
  
      void HandleMethodCall(
          const flutter::MethodCall<flutter::EncodableValue>& method_call,
          std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
          
          const auto& method = method_call.method_name();
          const auto& args = *method_call.arguments();
  
          try {
              if (method == "initPath") {
                  std::wstring path(args.StringValue());
                  int id = exifHandler->initPath(path);
                  result->Success(flutter::EncodableValue(id));
              }
              else if (method == "getAttribute") {
                  auto id = std::get<int>(args.Map().at(flutter::EncodableValue("id")));
                  std::wstring tag(args.Map().at(flutter::EncodableValue("tag")).StringValue());
                  std::wstring value = exifHandler->getAttribute(id, tag);
                  result->Success(flutter::EncodableValue(value));
              }
              else if (method == "getAttributes") {
                  auto id = std::get<int>(args.Map().at(flutter::EncodableValue("id")));
                  auto attributes = exifHandler->getAttributes(id);
                  flutter::EncodableMap resultMap;
                  for (const auto& pair : attributes) {
                      resultMap[flutter::EncodableValue(pair.first)] = flutter::EncodableValue(pair.second);
                  }
                  result->Success(flutter::EncodableValue(resultMap));
              }
              else if (method == "setAttribute") {
                  auto id = std::get<int>(args.Map().at(flutter::EncodableValue("id")));
                  std::wstring tag(args.Map().at(flutter::EncodableValue("tag")).StringValue());
                  std::wstring value(args.Map().at(flutter::EncodableValue("value")).StringValue());
                  bool success = exifHandler->setAttribute(id, tag, value);
                  result->Success(flutter::EncodableValue(success));
              }
              else if (method == "close") {
                  auto id = std::get<int>(args.Map().at(flutter::EncodableValue("id")));
                  exifHandler->close(id);
                  result->Success();
              }
              else {
                  result->NotImplemented();
              }
          } catch (const std::exception& e) {
              result->Error("ERROR", e.what());
          }
      }
  };
  
  extern "C" __declspec(dllexport) void NativeExifRegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar) {
      NativeExifPlugin::RegisterWithRegistrar(
          flutter::PluginRegistrarManager::GetInstance()
              ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
  }