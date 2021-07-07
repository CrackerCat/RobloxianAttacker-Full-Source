#pragma once
#include <Windows.h>
#include <math.h>
#include <string>
#include <locale>
#include <no_scanner.h>
#include <iostream>
#include <no_inlinesyscalls.h>
#include <no_winapi.h>

#define RVA(addr, size) ((PBYTE)((UINT_PTR)(addr) + *(PINT)((UINT_PTR)(addr) + ((size) - sizeof(INT))) + (size)))

void functions_get_text_size(const char* text, int& width, int& height) noexcept
{
	int w_i_d_t_h = 0;
	std::string stringArray[80];

	for (int i = 0; i < strlen(text); i++) {
		stringArray[i] = std::string(text);

		if (stringArray[i] == "i" or stringArray[i] == "t" or stringArray[i] == "r")
			w_i_d_t_h = w_i_d_t_h + 3;
		else
			w_i_d_t_h = w_i_d_t_h + 7;

	}

	width = w_i_d_t_h;
	height = 13;
}

namespace SDK
{
	namespace Structs
	{
		template<class T>
		struct TArray
		{
			friend struct FString;

		public:
			inline TArray()
			{
				Data = nullptr;
				Count = Max = 0;
			};

			inline int Num() const
			{
				return Count;
			};

			inline T& operator[](int i)
			{
				return Data[i];
			};

			inline const T& operator[](int i) const
			{
				return Data[i];
			};

			inline bool IsValidIndex(int i) const
			{
				return i < Num();
			}


			T* Data;
		private:
			int32_t Count;
			int32_t Max;
		};


		struct FString : private TArray<wchar_t>
		{
			inline FString()
			{
			};

			FString(const wchar_t* other)
			{
				Max = Count = *other ? wcslen(other) + 1 : 0;

				if (Count)
				{
					Data = const_cast<wchar_t*>(other);
				}
			};

			inline bool IsValid() const
			{
				return Data != nullptr;
			}

			inline const wchar_t* c_str() const
			{
				return Data;
			}

			std::string ToString() const
			{
				auto length = std::wcslen(Data);

				std::string str(length, '\0');

				std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

				return str;
			}
		};

		struct FVector2D
		{
			FVector2D() : x(0.f), y(0.f)
			{

			}

			FVector2D(float _x, float _y) : x(_x), y(_y)
			{

			}
			~FVector2D()
			{

			}
			float x, y;
		};

		class Vector3
		{
		public:
			Vector3() : x(0.f), y(0.f), z(0.f)
			{

			}

			Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
			{

			}
			~Vector3()
			{

			}

			float x;
			float y;
			float z;

			float Dot(Vector3 v)
			{
				return x * v.x + y * v.y + z * v.z;
			}

			float Distance(Vector3 v)
			{
				auto A = powf_((float)v.x - x, (float)2.0);
				auto B = powf_((float)v.y - y, (float)2.0);
				auto C = powf_((float)v.z - z, (float)2.0);
				return float(sqrtf_(A + B + C));
			}

			Vector3 ClampAngles() {

				if (x < -180.0f)
					x += 360.0f;

				if (x > 180.0f)
					x -= 360.0f;

				if (y < -74.0f)
					y = -74.0f;

				if (y > 74.0f)
					y = 74.0f;

				return Vector3(x, y, 0);
			}

			double Length() {
				return sqrtf_(x * x + y * y + z * z);
			}

			Vector3 operator+(Vector3 v)
			{
				return Vector3(x + v.x, y + v.y, z + v.z);
			}

			Vector3 operator-(Vector3 v)
			{
				return Vector3(x - v.x, y - v.y, z - v.z);
			}

			Vector3 operator*(float flNum) { return Vector3(x * flNum, y * flNum, z * flNum); }
		};

		struct FName
		{
			int32_t ComparisonIndex;
			int32_t Number;
		};

		struct FBox
		{
			Vector3  Min;
			Vector3  Max;
			unsigned char IsValid;
			unsigned char UnknownData00[0x3];
		};

		class UClass {
		public:
			BYTE _padding_0[0x40];
			UClass* SuperClass;
		};

		class UObject {
		public:
			PVOID VTableObject;  // 0x0
			DWORD ObjectFlags;   // 0x8
			DWORD InternalIndex; // 0xC
			UClass* Class;		 // 0x14
			FName Names;
			UObject* Outer;		 // 0x24

			inline void ProcessEvent(void* fn, void* parms)
			{
				auto vtable = *reinterpret_cast<void***>(this);
				auto processEventFn = static_cast<void(*)(void*, void*, void*)>(vtable[0x44]);
				processEventFn(this, fn, parms);
			}

			inline BOOLEAN IsA(PVOID parentClass) {
				for (auto super = this->Class; super; super = super->SuperClass) {
					if (super == parentClass) {
						return TRUE;
					}
				}

				return FALSE;
			}
		};

		class FUObjectItem {
		public:
			UObject* Object;
			DWORD Flags;
			DWORD ClusterIndex;
			DWORD SerialNumber;
			DWORD SerialNumber2;
		};

		class TUObjectArray {
		public:
			FUObjectItem* Objects[9];
		};

		class GObjects {
		public:
			TUObjectArray* ObjectArray;
			BYTE _padding_0[0xC];
			DWORD ObjectCount;
		};

		void freememory(uintptr_t Ptr, int Length = 8)
		{
			SpoofCall(VirtualFree, (LPVOID)Ptr, (SIZE_T)0, (DWORD)MEM_RELEASE);
		}

		template <typename T>
		std::string GetObjectName(T Object, bool GetOuter = false)
		{
			static uintptr_t GetObjectName;

			if (!GetObjectName)
				GetObjectName = sigscan(Fortnite, _("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 48 8B F2 44 39 71 04 0F 85 ? ? ? ? 8B 19 0F B7 FB E8 ? ? ? ? 8B CB 48 8D 54 24"));

			auto UFUNGetObjectNameByIndex = reinterpret_cast<FString * (__fastcall*)(DWORD*, FString*)>(GetObjectName);

			DWORD ObjectIndex = *(DWORD*)((PBYTE)Object + 0x18);
			if (!ObjectIndex || ObjectIndex >= MAXDWORD) return _("");

			if (!GetOuter)
			{
				FString A;
				std::string B = "";
				SpoofCall(UFUNGetObjectNameByIndex, &ObjectIndex, &A);
				B = A.ToString();
				return B;
			}

			std::string Ret;

			UObject* Object_ = (UObject*)Object;
			for (int i = 0; Object_; Object_ = Object_->Outer, i++)
			{
				FString Result;

				SpoofCall(UFUNGetObjectNameByIndex, &ObjectIndex, &Result);

				std::string tmp = Result.ToString();

				if (tmp.c_str() == 0) return _("");

				freememory((__int64)Result.c_str(), tmp.size() + 8);

				char return_string[1024] = {};
				for (size_t i = 0; i < tmp.size(); i++)
				{
					return_string[i] += tmp[i];
				}

				Ret = return_string + std::string(i > 0 ? "." : "") + Ret;
			}

			return Ret;
		}

		bool IsNumber(char Character)
		{
			if (Character >= 48 && Character <= 57)
				return true;
		}

		void FixNames(char* BufferIn)
		{
			for (int i = 0; i < strlen(BufferIn); i++)
				if (BufferIn[i] == '_')
					if (IsNumber(BufferIn[i + 1]))
						BufferIn[i] = 0;
		}

		static UObject* get_object_with_id(int id, uint64_t GObjects)
		{
			if (GObjects == 0)
				return NULL;

			auto object = *(UObject**)(*(uint64_t*)(GObjects + 8i64 * (id / 0x10000)) + 24i64 * (id % 0x10000));

			if (object == NULL)
				return NULL;

			if (object->ObjectFlags <= 0)
				return NULL;

			return object;
		}

		GObjects* objects = nullptr;
		PVOID FindObject(std::string name) {

			bool once = false;
			if (!once)
			{
				auto UObjectPtr = sigscan(Fortnite, _("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1"));
				objects = decltype(SDK::Structs::objects)(RVA(UObjectPtr, 7));

				once = true;
			}

			for (auto array : objects->ObjectArray->Objects) {
				auto fuObject = array;
				for (auto i = 0; i < 0x10000 && fuObject->Object; ++i, ++fuObject) {
					auto object = fuObject->Object;
					if (object->ObjectFlags != 0x41)
						continue;

					if (strstr(GetObjectName(object).c_str(), name.c_str()))
						return object;
				}
			}
			return 0;
		}

		struct FLinearColor
		{
			FLinearColor() : R(0.f), G(0.f), B(0.f), A(0.f)
			{

			}

			FLinearColor(float _R, float _G, float _B, float _A) : R(_R), G(_G), B(_B), A(_A)
			{

			}

			float R; // 0x00(0x04)
			float G; // 0x04(0x04)
			float B; // 0x08(0x04)
			float A; // 0x0c(0x04)
		};

		struct FColor
		{
			uint8_t R; // 0x00(0x04)
			uint8_t G; // 0x04(0x04)
			uint8_t B; // 0x08(0x04)
			uint8_t A; // 0x0c(0x04)
		};

		enum class EFontCacheType : uint8_t
		{
			Offline = 0,
			Runtime = 1,
			EFontCacheType_MAX = 2
		};

		enum class EFontImportCharacterSet : uint8_t
		{
			FontICS_Default = 0,
			FontICS_Ansi = 1,
			FontICS_Symbol = 2,
			FontICS_MAX = 3
		};

		template<class TEnum>
		class TEnumAsByte
		{
		public:
			inline TEnumAsByte()
			{
			}

			inline TEnumAsByte(TEnum _value)
				: value(static_cast<uint8_t>(_value))
			{
			}

			explicit inline TEnumAsByte(int32_t _value)
				: value(static_cast<uint8_t>(_value))
			{
			}

			explicit inline TEnumAsByte(uint8_t _value)
				: value(_value)
			{
			}

			inline operator TEnum() const
			{
				return (TEnum)value;
			}

			inline TEnum GetValue() const
			{
				return (TEnum)value;
			}

		private:
			uint8_t value;
		};

		struct FFontImportOptionsData
		{
			struct FString                                     FontName;                                                 // 0x0000(0x0010) (Edit, ZeroConstructor)
			float                                              Height;                                                   // 0x0010(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      bEnableAntialiasing : 1;                                  // 0x0014(0x0001) (Edit)
			unsigned char                                      bEnableBold : 1;                                          // 0x0014(0x0001) (Edit)
			unsigned char                                      bEnableItalic : 1;                                        // 0x0014(0x0001) (Edit)
			unsigned char                                      bEnableUnderline : 1;                                     // 0x0014(0x0001) (Edit)
			unsigned char                                      bAlphaOnly : 1;                                           // 0x0014(0x0001) (Edit)
			unsigned char                                      UnknownData00[0x3];                                       // 0x0015(0x0003) MISSED OFFSET
			TEnumAsByte<EFontImportCharacterSet>               CharacterSet;                                             // 0x0018(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      UnknownData01[0x7];                                       // 0x0019(0x0007) MISSED OFFSET
			struct FString                                     Chars;                                                    // 0x0020(0x0010) (Edit, ZeroConstructor)
			struct FString                                     UnicodeRange;                                             // 0x0030(0x0010) (Edit, ZeroConstructor)
			struct FString                                     CharsFilePath;                                            // 0x0040(0x0010) (Edit, ZeroConstructor)
			struct FString                                     CharsFileWildcard;                                        // 0x0050(0x0010) (Edit, ZeroConstructor)
			unsigned char                                      bCreatePrintableOnly : 1;                                 // 0x0060(0x0001) (Edit)
			unsigned char                                      bIncludeASCIIRange : 1;                                   // 0x0060(0x0001) (Edit)
			unsigned char                                      UnknownData02[0x3];                                       // 0x0061(0x0003) MISSED OFFSET
			struct FLinearColor                                ForegroundColor;                                          // 0x0064(0x0010) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      bEnableDropShadow : 1;                                    // 0x0074(0x0001) (Edit)
			unsigned char                                      UnknownData03[0x3];                                       // 0x0075(0x0003) MISSED OFFSET
			int                                                TexturePageWidth;                                         // 0x0078(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                TexturePageMaxHeight;                                     // 0x007C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                XPadding;                                                 // 0x0080(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                YPadding;                                                 // 0x0084(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                ExtendBoxTop;                                             // 0x0088(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                ExtendBoxBottom;                                          // 0x008C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                ExtendBoxRight;                                           // 0x0090(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                ExtendBoxLeft;                                            // 0x0094(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      bEnableLegacyMode : 1;                                    // 0x0098(0x0001) (Edit)
			unsigned char                                      UnknownData04[0x3];                                       // 0x0099(0x0003) MISSED OFFSET
			int                                                Kerning;                                                  // 0x009C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      bUseDistanceFieldAlpha : 1;                               // 0x00A0(0x0001) (Edit)
			unsigned char                                      UnknownData05[0x3];                                       // 0x00A1(0x0003) MISSED OFFSET
			int                                                DistanceFieldScaleFactor;                                 // 0x00A4(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			float                                              DistanceFieldScanRadiusScale;                             // 0x00A8(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      UnknownData06[0x4];                                       // 0x00AC(0x0004) MISSED OFFSET
		};

		struct FTypeface
		{
			TArray<struct FTypefaceEntry>                      Fonts;                                                    // 0x0000(0x0010) (ZeroConstructor)
		};

		struct FCompositeFallbackFont
		{
			struct FTypeface                                   Typeface;                                                 // 0x0000(0x0010)
			float                                              ScalingFactor;                                            // 0x0010(0x0004) (ZeroConstructor, IsPlainOldData)
			unsigned char                                      UnknownData00[0x4];                                       // 0x0014(0x0004) MISSED OFFSET
		};

		struct FCompositeFont
		{
			struct FTypeface                                   DefaultTypeface;                                          // 0x0000(0x0010)
			struct FCompositeFallbackFont                      FallbackTypeface;                                         // 0x0010(0x0018)
			TArray<struct FCompositeSubFont>                   SubTypefaces;                                             // 0x0028(0x0010) (ZeroConstructor)
		};

		class UFont : public UObject
		{
		public:
			unsigned char                                      UnknownData00[0x8];                                       // 0x0028(0x0008) MISSED OFFSET
			EFontCacheType                                     FontCacheType;                                            // 0x0030(0x0001) (Edit, ZeroConstructor, IsPlainOldData)
			unsigned char                                      UnknownData01[0x7];                                       // 0x0031(0x0007) MISSED OFFSET
			TArray<struct FFontCharacter>                      Characters;                                               // 0x0038(0x0010) (Edit, ZeroConstructor)
			TArray<class UTexture2D*>                          Textures;                                                 // 0x0048(0x0010) (ZeroConstructor)
			int                                                IsRemapped;                                               // 0x0058(0x0004) (ZeroConstructor, IsPlainOldData)
			float                                              EmScale;                                                  // 0x005C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			float                                              Ascent;                                                   // 0x0060(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			float                                              Descent;                                                  // 0x0064(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			float                                              Leading;                                                  // 0x0068(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                Kerning;                                                  // 0x006C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			struct FFontImportOptionsData                      ImportOptions;                                            // 0x0070(0x00B0) (Edit)
			int                                                NumCharacters;                                            // 0x0120(0x0004) (ZeroConstructor, Transient, IsPlainOldData)
			unsigned char                                      UnknownData02[0x4];                                       // 0x0124(0x0004) MISSED OFFSET
			TArray<int>                                        MaxCharHeight;                                            // 0x0128(0x0010) (ZeroConstructor, Transient)
			float                                              ScalingFactor;                                            // 0x0138(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			int                                                LegacyFontSize;                                           // 0x013C(0x0004) (Edit, ZeroConstructor, IsPlainOldData)
			struct FName                                       LegacyFontName;                                           // 0x0140(0x0008) (Edit, ZeroConstructor, IsPlainOldData)
			struct FCompositeFont                              CompositeFont;                                            // 0x0148(0x0038)
			unsigned char                                      UnknownData03[0x50];                                      // 0x0180(0x0050) MISSED OFFSET
		};

		struct UCanvas_K2_DrawText_Params
		{
			class SDK::Structs::UFont* RenderFont;                                               // (Parm, ZeroConstructor, IsPlainOldData)
			struct FString                                     RenderText;                                               // (Parm, ZeroConstructor)
			struct FVector2D                                   ScreenPosition;                                           // (Parm, ZeroConstructor, IsPlainOldData)
			struct FVector2D                                   Scale;                                                    // (Parm, ZeroConstructor, IsPlainOldData)
			struct FLinearColor                                RenderColor;                                              // (Parm, ZeroConstructor, IsPlainOldData)
			float                                              Kerning;                                                  // (Parm, ZeroConstructor, IsPlainOldData)
			struct FLinearColor                                ShadowColor;                                              // (Parm, ZeroConstructor, IsPlainOldData)
			struct FVector2D                                   ShadowOffset;                                             // (Parm, ZeroConstructor, IsPlainOldData)
			bool                                               bCentreX;                                                 // (Parm, ZeroConstructor, IsPlainOldData)
			bool                                               bCentreY;                                                 // (Parm, ZeroConstructor, IsPlainOldData)
			bool                                               bOutlined;                                                // (Parm, ZeroConstructor, IsPlainOldData)
			struct FLinearColor                                OutlineColor;                                             // (Parm, ZeroConstructor, IsPlainOldData)
		};

		class UField : public UObject
		{
		public:
			class UField* Next;                                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
		};

		// Class CoreUObject.Struct
		// 0x0068 (0x0098 - 0x0030)
		class UStruct : public UField
		{
		public:
			class UStruct* SuperField;                                               // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			class UField* Children;                                                 // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int32_t                                            PropertySize;                                             // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int32_t                                            MinAlignment;                                             // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			unsigned char                                      UnknownData0x0048[0x40];                                  // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
		};

		// Class CoreUObject.Function
		// 0x0030 (0x00C8 - 0x0098)
		class UFunction : public UStruct
		{
		public:

			int32_t                                            FunctionFlags;                                            // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int16_t                                            RepOffset;                                                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int8_t                                             NumParms;                                                 // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int16_t                                            ParmsSize;                                                // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int16_t                                            ReturnValueOffset;                                        // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int16_t                                            RPCId;                                                    // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int16_t                                            RPCResponseId;                                            // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			class UProperty* FirstPropertyToInit;                                      // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			class UFunction* EventGraphFunction;                                       // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			int32_t                                            EventGraphCallOffset;                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
			unsigned char buff[0x8];
			void* Func;                                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
		};

		enum eBone
		{
			BONE_NULL_1 = 0,
			BONE_NULL_2 = 1,
			BONE_PELVIS_1 = 2,
			BONE_PELVIS_2 = 3,
			BONE_PELVIS_3 = 4,
			BONE_TORSO = 5,

			BONE_CHEST_LOW = 6,
			BONE_CHEST = 7,

			// -------------------------

			BONE_CHEST_LEFT = 8,

			BONE_L_SHOULDER_1 = 9,
			BONE_L_ELBOW = 10,

			BONE_L_HAND_ROOT_1 = 11,

			BONE_L_FINGER_1_ROOT = 12,
			BONE_L_FINGER_1_LOW = 13,
			BONE_L_FINGER_1 = 14,
			BONE_L_FINGER_1_TOP = 15,

			BONE_L_FINGER_2_ROOT = 16,
			BONE_L_FINGER_2_LOW = 17,
			BONE_L_FINGER_2 = 18,
			BONE_L_FINGER_2_TOP = 19,

			BONE_L_FINGER_3_ROOT = 20,
			BONE_L_FINGER_3_LOW = 21,
			BONE_L_FINGER_3 = 22,
			BONE_L_FINGER_3_TOP = 23,

			BONE_L_FINGER_4_ROOT = 24,
			BONE_L_FINGER_4_LOW = 25,
			BONE_L_FINGER_4_ = 26,
			BONE_L_FINGER_4_TOP = 27,

			BONE_L_THUMB_ROOT = 28,
			BONE_L_THUMB_LOW = 29,
			BONE_L_THUMB = 30,

			BONE_L_HAND_ROOT_2 = 31,
			BONE_L_WRIST = 32,
			BONE_L_ARM_LOWER = 33,

			BONE_L_SHOULDER_2 = 34,

			BONE_L_ARM_TOP = 35,

			// -------------------------

			BONE_CHEST_TOP_1 = 36,

			// -------------------------

			BONE_CHEST_RIGHT = 37,

			BONE_R_ELBOW = 38,

			BONE_R_HAND_ROOT_1 = 39,

			BONE_R_FINGER_1_ROOT = 40,
			BONE_R_FINGER_1_LOW = 41,
			BONE_R_FINGER_1 = 42,
			BONE_R_FINGER_1_TOP = 43,

			BONE_R_FINGER_2_ROOT = 44,
			BONE_R_FINGER_2_LOW = 45,
			BONE_R_FINGER_2 = 46,
			BONE_R_FINGER_2_TOP = 47,

			BONE_R_FINGER_3_ROOT = 48,
			BONE_R_FINGER_3_LOW = 49,
			BONE_R_FINGER_3 = 50,
			BONE_R_FINGER_3_TOP = 51,

			BONE_R_FINGER_4_ROOT = 52,
			BONE_R_FINGER_4_LOW = 53,
			BONE_R_FINGER_4_ = 54,
			BONE_R_FINGER_4_TOP = 55,

			BONE_R_THUMB_ROOT = 56,
			BONE_R_THUMB_LOW = 57,
			BONE_R_THUMB = 58,

			BONE_R_HAND_ROOT = 59,
			BONE_R_WRIST = 60,
			BONE_R_ARM_LOWER = 61,

			BONE_R_SHOULDER = 62,

			BONE_R_ARM_TOP = 63,

			// -------------------------

			BONE_CHEST_TOP_2 = 64,

			BONE_NECK = 65,
			BONE_HEAD = 66,

			// -------------------------

			BONE_L_LEG_ROOT = 67,
			BONE_L_KNEE = 68,
			BONE_L_FOOT_ROOT = 69,
			BONE_L_SHIN = 70,
			BONE_L_FOOT_MID = 71,
			BONE_L_FOOT_LOW = 72,
			BONE_L_THIGH = 73,

			// -------------------------

			BONE_R_LEG_ROOT = 74,
			BONE_R_KNEE = 75,
			BONE_R_FOOT_ROOT = 76,
			BONE_R_SHIN = 77,
			BONE_R_FOOT_MID = 78,
			BONE_R_FOOT_LOW = 79,
			BONE_R_THIGH = 80,

			// -------------------------

			BONE_NULL_3 = 81,
			BONE_MISC_L_FOOT = 82,
			BONE_MISC_R_FOOT = 83,
			BONE_NULL_4 = 84,
			BONE_MISC_R_HAND_1 = 85,
			BONE_MISC_L_HAND = 86,
			BONE_MISC_R_HAND_2 = 87,
		};


		typedef struct _UNICODE_STRING {
			USHORT Length;
			USHORT MaximumLength;
#ifdef MIDL_PASS
			[size_is(MaximumLength / 2), length_is((Length) / 2)] USHORT* Buffer;
#else // MIDL_PASS
			_Field_size_bytes_part_opt_(MaximumLength, Length) PWCH   Buffer;
#endif // MIDL_PASS
		} UNICODE_STRING;
		typedef UNICODE_STRING* PUNICODE_STRING;
		typedef const UNICODE_STRING* PCUNICODE_STRING;

		typedef struct _PEB_LDR_DATA
		{
			ULONG Length;
			UCHAR Initialized;
			PVOID SsHandle;
			LIST_ENTRY InLoadOrderModuleList;
			LIST_ENTRY InMemoryOrderModuleList;
			LIST_ENTRY InInitializationOrderModuleList;
			PVOID EntryInProgress;
		} PEB_LDR_DATA, * PPEB_LDR_DATA;

		typedef struct _PEB
		{
			UCHAR InheritedAddressSpace;
			UCHAR ReadImageFileExecOptions;
			UCHAR BeingDebugged;
			UCHAR BitField;
			ULONG ImageUsesLargePages : 1;
			ULONG IsProtectedProcess : 1;
			ULONG IsLegacyProcess : 1;
			ULONG IsImageDynamicallyRelocated : 1;
			ULONG SpareBits : 4;
			PVOID Mutant;
			PVOID ImageBaseAddress;
			PPEB_LDR_DATA Ldr;
			VOID* ProcessParameters;
			PVOID SubSystemData;
			PVOID ProcessHeap;
			PRTL_CRITICAL_SECTION FastPebLock;
			PVOID AtlThunkSListPtr;
			PVOID IFEOKey;
			ULONG CrossProcessFlags;
			ULONG ProcessInJob : 1;
			ULONG ProcessInitializing : 1;
			ULONG ReservedBits0 : 30;
			union
			{
				PVOID KernelCallbackTable;
				PVOID UserSharedInfoPtr;
			};
			ULONG SystemReserved[1];
			ULONG SpareUlong;
			VOID* FreeList;
			ULONG TlsExpansionCounter;
			PVOID TlsBitmap;
			ULONG TlsBitmapBits[2];
			PVOID ReadOnlySharedMemoryBase;
			PVOID HotpatchInformation;
			VOID** ReadOnlyStaticServerData;
			PVOID AnsiCodePageData;
			PVOID OemCodePageData;
			PVOID UnicodeCaseTableData;
			ULONG NumberOfProcessors;
			ULONG NtGlobalFlag;
			LARGE_INTEGER CriticalSectionTimeout;
			ULONG HeapSegmentReserve;
			ULONG HeapSegmentCommit;
			ULONG HeapDeCommitTotalFreeThreshold;
			ULONG HeapDeCommitFreeBlockThreshold;
			ULONG NumberOfHeaps;
			ULONG MaximumNumberOfHeaps;
			VOID** ProcessHeaps;
			PVOID GdiSharedHandleTable;
			PVOID ProcessStarterHelper;
			ULONG GdiDCAttributeList;
			PRTL_CRITICAL_SECTION LoaderLock;
			ULONG OSMajorVersion;
			ULONG OSMinorVersion;
			WORD OSBuildNumber;
			WORD OSCSDVersion;
			ULONG OSPlatformId;
			ULONG ImageSubsystem;
			ULONG ImageSubsystemMajorVersion;
			ULONG ImageSubsystemMinorVersion;
			ULONG ImageProcessAffinityMask;
			ULONG GdiHandleBuffer[34];
			PVOID PostProcessInitRoutine;
			PVOID TlsExpansionBitmap;
			ULONG TlsExpansionBitmapBits[32];
			ULONG SessionId;
			ULARGE_INTEGER AppCompatFlags;
			ULARGE_INTEGER AppCompatFlagsUser;
			PVOID pShimData;
			PVOID AppCompatInfo;
			UNICODE_STRING CSDVersion;
			VOID* ActivationContextData;
			VOID* ProcessAssemblyStorageMap;
			VOID* SystemDefaultActivationContextData;
			VOID* SystemAssemblyStorageMap;
			ULONG MinimumStackCommit;
			VOID* FlsCallback;
			LIST_ENTRY FlsListHead;
			PVOID FlsBitmap;
			ULONG FlsBitmapBits[4];
			ULONG FlsHighIndex;
			PVOID WerRegistrationData;
			PVOID WerShipAssertPtr;
		} PEB, * PPEB;
	}

	namespace Utilities
	{
		HMODULE GetImageBase()
		{
			return (HMODULE)(((Structs::PPEB)__readgsqword(0x60))->ImageBaseAddress);
		}
	}
}