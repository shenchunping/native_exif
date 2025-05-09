#include <wincodec.h>
#include <wincodecsdk.h>
#include <map>
#include <string>
#include <comdef.h>
#include <atlbase.h>
#include <vector>

class NativeExifWindows {
private:
    std::map<int, CComPtr<IWICMetadataQueryReader>> readers;
    std::map<int, CComPtr<IWICMetadataQueryWriter>> writers;
    std::map<int, std::wstring> filePaths;
    std::map<int, CComPtr<IWICBitmapDecoder>> decoders;
    int nextId = 0;
    CComPtr<IWICImagingFactory> pFactory;

    HRESULT InitializeWIC() {
        return CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pFactory)
        );
    }

public:
    NativeExifWindows() {
        CoInitialize(NULL);
        InitializeWIC();
    }

    ~NativeExifWindows() {
        CoUninitialize();
    }

    int initPath(const std::wstring& filePath) {
        CComPtr<IWICBitmapDecoder> pDecoder;
        HRESULT hr = pFactory->CreateDecoderFromFilename(
            filePath.c_str(),
            NULL,
            GENERIC_READ | GENERIC_WRITE,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
        );

        if (SUCCEEDED(hr)) {
            CComPtr<IWICMetadataQueryReader> pQueryReader;
            hr = pDecoder->GetMetadataQueryReader(&pQueryReader);
            
            if (SUCCEEDED(hr)) {
                int id = nextId++;
                readers[id] = pQueryReader;
                filePaths[id] = filePath;
                decoders[id] = pDecoder;
                
                // 创建写入器
                CComPtr<IWICMetadataQueryWriter> pQueryWriter;
                pDecoder->GetMetadataQueryWriter(&pQueryWriter);
                if (pQueryWriter) {
                    writers[id] = pQueryWriter;
                }
                return id;
            }
        }
        return -1;
    }

    std::wstring getAttribute(int id, const std::wstring& tag) {
        if (readers.find(id) == readers.end()) return L"";

        PROPVARIANT value;
        PropVariantInit(&value);
        
        HRESULT hr = readers[id]->GetMetadataByName(tag.c_str(), &value);
        if (SUCCEEDED(hr)) {
            std::wstring result;
            switch (value.vt) {
                case VT_LPWSTR: result = value.pwszVal; break;
                case VT_UI2: result = std::to_wstring(value.uiVal); break;
                case VT_UI4: result = std::to_wstring(value.ulVal); break;
                case VT_R8: result = std::to_wstring(value.dblVal); break;
                default: result = L"";
            }
            PropVariantClear(&value);
            return result;
        }
        return L"";
    }

    std::map<std::wstring, std::wstring> getAttributes(int id) {
        std::map<std::wstring, std::wstring> attributes;
        if (readers.find(id) == readers.end()) return attributes;

        // 常见EXIF标签
        std::vector<std::wstring> tags = {
            L"System.Photo.Aperture",
            L"System.Photo.DateTimeDigitized",
            L"System.Photo.DateTime",
            L"System.Photo.ExposureTime",
            L"System.Photo.FNumber",
            L"System.Photo.Flash",
            L"System.Photo.FocalLength",
            L"System.Photo.ISOSpeed",
            L"System.Photo.LensManufacturer",
            L"System.Photo.LensModel",
            L"System.Photo.Maker",
            L"System.Photo.Model",
            L"System.Photo.Orientation",
            L"System.Photo.ShutterSpeed",
            L"System.GPS.Latitude",
            L"System.GPS.Longitude",
            L"System.GPS.Altitude",
            L"System.Image.HorizontalSize",
            L"System.Image.VerticalSize"
        };

        for (const auto& tag : tags) {
            std::wstring value = getAttribute(id, tag);
            if (!value.empty()) {
                attributes[tag] = value;
            }
        }

        return attributes;
    }

    bool setAttribute(int id, const std::wstring& tag, const std::wstring& value) {
        if (writers.find(id) == writers.end()) return false;

        PROPVARIANT propValue;
        PropVariantInit(&propValue);
        propValue.vt = VT_LPWSTR;
        propValue.pwszVal = const_cast<wchar_t*>(value.c_str());

        HRESULT hr = writers[id]->SetMetadataByName(tag.c_str(), &propValue);
        PropVariantClear(&propValue);
        return SUCCEEDED(hr);
    }

    bool setGpsCoordinates(int id, double latitude, double longitude) {
        if (writers.find(id) == writers.end()) return false;

        // 设置GPS纬度
        setAttribute(id, L"System.GPS.Latitude", std::to_wstring(abs(latitude)));
        setAttribute(id, L"System.GPS.LatitudeRef", latitude >= 0 ? L"N" : L"S");
        
        // 设置GPS经度
        setAttribute(id, L"System.GPS.Longitude", std::to_wstring(abs(longitude)));
        setAttribute(id, L"System.GPS.LongitudeRef", longitude >= 0 ? L"E" : L"W");
        
        return true;
    }

    bool saveChanges(int id) {
        if (decoders.find(id) == decoders.end()) return false;

        CComPtr<IWICBitmapEncoder> pEncoder;
        HRESULT hr = pFactory->CreateEncoder(
            GUID_ContainerFormatJpeg,
            NULL,
            &pEncoder
        );

        if (SUCCEEDED(hr)) {
            hr = pEncoder->Initialize(decoders[id], WICBitmapEncoderNoCache);
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Commit();
                return SUCCEEDED(hr);
            }
        }
        return false;
    }

    void close(int id) {
        readers.erase(id);
        writers.erase(id);
        filePaths.erase(id);
        decoders.erase(id);
    }
};