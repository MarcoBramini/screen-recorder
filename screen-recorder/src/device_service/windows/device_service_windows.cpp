#include <dshow.h>
#include <windows.h>
#include <vector>
#include "../../../include/device_service.h"

#include <comutil.h>
#include <stdio.h>

#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "strmiids")

HRESULT EnumerateDevices(REFGUID category, IEnumMoniker** ppEnum) {
  // Create the System Device Enumerator.
  ICreateDevEnum* pDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

  if (SUCCEEDED(hr)) {
    // Create an enumerator for the category.
    hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
    if (hr == S_FALSE) {
      hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    }
    pDevEnum->Release();
  }
  return hr;
}

std::vector<InputDeviceVideo> DeviceService::get_input_video_devices() {
  std::vector<InputDeviceVideo> devices;
  devices.emplace_back("desktop", "gdigrab", "Desktop", 0.0f, 0.0f, 0.0f, 0.0f,
                       true, "0");

  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    IEnumMoniker* pEnum;

    hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr)) {
      IMoniker* pMoniker = NULL;

      while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
        IPropertyBag* pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr)) {
          pMoniker->Release();
          continue;
        }

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"FriendlyName", &var, 0);
        if (SUCCEEDED(hr)) {
          printf("%S\n", var.bstrVal);

          devices.emplace_back(
              fmt::format("video=\"{}\"",
                          _com_util::ConvertBSTRToString(var.bstrVal)),
              "dshow", _com_util::ConvertBSTRToString(var.bstrVal), 0.0f, 0.0f,
              0.0f, 0.0f, true, "0");
          VariantClear(&var);
        } else {
          pMoniker->Release();
          continue;
        }
        pPropBag->Release();
        pMoniker->Release();
      }

      pEnum->Release();
    }
    CoUninitialize();
  }
  return devices;
}

std::vector<InputDeviceAudio> DeviceService::get_input_audio_devices() {
  std::vector<InputDeviceAudio> devices;

  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    IEnumMoniker* pEnum;

    hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr)) {
      IMoniker* pMoniker = NULL;

      while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
        IPropertyBag* pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr)) {
          pMoniker->Release();
          continue;
        }

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"FriendlyName", &var, 0);
        if (SUCCEEDED(hr)) {
          devices.emplace_back(
              fmt::format("audio=\"{}\"",
                          _com_util::ConvertBSTRToString(var.bstrVal)),
              "dshow", _com_util::ConvertBSTRToString(var.bstrVal));
          VariantClear(&var);
        } else {
          pMoniker->Release();
          continue;
        }
        pPropBag->Release();
        pMoniker->Release();
      }

      pEnum->Release();
    }
    CoUninitialize();
  }

  return devices;
}

PermissionsStatus DeviceService::check_permissions() {
  return {.screenCaptureAllowed = true,
          .microphoneCaptureAllowed = true,
          .cameraCaptureAllowed = true};
}

void DeviceService::setup_screen_capture_permission(
    std::function<void(bool)> onComplete) {}

void DeviceService::setup_microphone_usage_permission(
    std::function<void(bool)> onComplete) {}

void DeviceService::setup_camera_usage_permission(
    std::function<void(bool)> onComplete) {}
