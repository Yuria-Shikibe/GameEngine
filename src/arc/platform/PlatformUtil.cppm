module;

#if defined(_WIN32)
#include <Windows.h>

#include <thumbcache.h>  // For IThumbnailProvider
#include <shobjidl.h>    // For IShellItem
#include <gdiplus.h>

#endif

export module ext.platform;
import std;
import OS.File;
import Graphic.Pixmap;

#ifdef _WIN32
namespace CurrentPlatform{
	std::vector<OS::File> getLogicalDrives(){
		std::vector<OS::File> rst{};
		const DWORD drives = GetLogicalDrives();
		for (int i = 0; i < 26; ++i) {
			if (drives & 1 << i) {
				const std::array arr{static_cast<char>('A' + i), ':', '\\', '\\', '\0'};
				rst.emplace_back(arr.data());
			}
		}

		return rst;
	}

	void PrintErrorMessage(const HRESULT hr) {
		if (FAILED(hr)) {
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError();

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			   nullptr,
				hr,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				reinterpret_cast<LPTSTR>(&lpMsgBuf),
				0, nullptr);

			std::wcout << static_cast<LPCTSTR>(lpMsgBuf) << std::endl;

			LocalFree(lpMsgBuf);
		}
	}

	Graphic::Pixmap getThumbnail(const OS::File& file) {
		Graphic::Pixmap pixmap{4, 4};

		IShellItemImageFactory *pImgFactory = nullptr;

		if(const auto result = SHCreateItemFromParsingName(file.getPath().wstring().data(), nullptr, IID_PPV_ARGS(&pImgFactory));
			FAILED(result)){
			PrintErrorMessage(result);
		}else{
			const SIZE size = { 256, 256 }; // Replace with your desired size
			HBITMAP hBitmap = nullptr;

			if(const auto hr = pImgFactory->GetImage(size, SIIGBF_RESIZETOFIT, &hBitmap);
				FAILED(hr)){
				PrintErrorMessage(hr);
			}else{
				Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, nullptr);

				Gdiplus::BitmapData bitmapData{};
				const Gdiplus::Rect rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
				pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);

				pixmap.create(pBitmap->GetWidth(), pBitmap->GetHeight());

				auto toRead = static_cast<Graphic::Pixmap::DataType*>(bitmapData.Scan0);
				for(int i = 0; i < pixmap.pixelSize(); ++i){
					pixmap[i * 4 + 0] = toRead[i * 4 + 2];
					pixmap[i * 4 + 1] = toRead[i * 4 + 1];
					pixmap[i * 4 + 2] = toRead[i * 4 + 0];
					pixmap[i * 4 + 3] = toRead[i * 4 + 3];
				}

				pixmap.flipY();

				delete pBitmap;
			}

			pImgFactory->Release();
		}

		return pixmap;
	}
}
#endif

export namespace ext::platform{
	inline std::vector<OS::File> getLogicalDrives(){
		return CurrentPlatform::getLogicalDrives();
	}

	inline Graphic::Pixmap getThumbnail(const OS::File& file){
		return CurrentPlatform::getThumbnail(file);
	}
}
