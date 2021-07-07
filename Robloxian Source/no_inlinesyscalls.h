#pragma once
#include <includes.h>

namespace SDK
{
	namespace Syscall
	{
		auto GetSyscallIndex(std::string ModuleName, std::string SyscallFunctionName, void* Function) -> bool
		{
			auto ModuleBaseAddress = LI_FN(GetModuleHandleA)(ModuleName.c_str());
			if (!ModuleBaseAddress)
				ModuleBaseAddress = LI_FN(LoadLibraryA)(ModuleName.c_str());
			if (!ModuleBaseAddress)
				return false;

			auto GetFunctionAddress = LI_FN(GetProcAddress)(ModuleBaseAddress, SyscallFunctionName.c_str());
			if (!GetFunctionAddress)
				return false;

			auto SyscallIndex = *(DWORD*)((PBYTE)GetFunctionAddress + 4);

			*(DWORD*)((PBYTE)Function + 4) = SyscallIndex;

			return true;
		}

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

		VOID RtlInitUnicodeString_(PUNICODE_STRING DestinationString, PCWSTR SourceString)
		{
			SIZE_T Size;
			CONST SIZE_T MaxSize = (USHRT_MAX & ~1) - sizeof(UNICODE_NULL); // an even number

			if (SourceString)
			{
				Size = wcslen(SourceString) * sizeof(WCHAR);
				__analysis_assume(Size <= MaxSize);

				if (Size > MaxSize)
					Size = MaxSize;
				DestinationString->Length = (USHORT)Size;
				DestinationString->MaximumLength = (USHORT)Size + sizeof(UNICODE_NULL);
			}
			else
			{
				DestinationString->Length = 0;
				DestinationString->MaximumLength = 0;
			}

			DestinationString->Buffer = (PWCHAR)SourceString;
		}

		extern "C"
		{
			NTSTATUS _NtUserSendInput(UINT a1, LPINPUT Input, int Size);
			SHORT _NtUserGetAsyncKeyState(int VKey);
			BOOL _ScreenToClient(HWND hWnd, LPPOINT lpPoint);
			BOOL _GetCursorPos(LPPOINT lpPoint);
		};

		BOOL GetCursorPos_(LPPOINT lpPoint)
		{
			static bool doneonce;
			if (!doneonce)
			{
				if (!GetSyscallIndex(_("User32.dll"), _("GetCursorPos"), _ScreenToClient))
					return 0;
				doneonce = true;
			}

			return _GetCursorPos(lpPoint);
		}

		BOOL ScreenToClient_(HWND hWnd, LPPOINT lpPoint)
		{
			static bool doneonce;
			if (!doneonce)
			{
				if (!GetSyscallIndex(_("User32.dll"), _("ScreenToClient"), _ScreenToClient))
					return 0;
				doneonce = true;
			}

			return _ScreenToClient(hWnd, lpPoint);
		}

		VOID mouse_event_(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo, bool autoshoot = true)
		{
			static bool doneonce;
			if (!doneonce)
			{
				if (!GetSyscallIndex(_("win32u.dll"), _("NtUserSendInput"), _NtUserSendInput))
					return;
				doneonce = true;
			}

			INPUT Input[3] = { 0 };
			Input[0].type = INPUT_MOUSE;
			Input[0].mi.dx = dx;
			Input[0].mi.dy = dy;
			Input[0].mi.mouseData = dwData;
			Input[0].mi.dwFlags = dwFlags;
			Input[0].mi.time = 0;
			Input[0].mi.dwExtraInfo = dwExtraInfo;

			if (autoshoot)
			{
				Input[1].type = INPUT_MOUSE;
				Input[2].type = INPUT_MOUSE;
				Input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				Input[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
				_NtUserSendInput((UINT)3, (LPINPUT)&Input, (INT)sizeof(INPUT));
			}
			else
				_NtUserSendInput((UINT)1, (LPINPUT)&Input, (INT)sizeof(INPUT));
		}

		void mousemove(float tarx, float tary, float X, float Y, int smooth, bool autoshoot)
		{
			float ScreenCenterX = (X / 2);
			float ScreenCenterY = (Y / 2);
			float TargetX = 0;
			float TargetY = 0;

			if (tarx != 0)
			{
				if (tarx > ScreenCenterX)
				{
					TargetX = -(ScreenCenterX - tarx);
					TargetX /= smooth;
					if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
				}

				if (tarx < ScreenCenterX)
				{
					TargetX = tarx - ScreenCenterX;
					TargetX /= smooth;
					if (TargetX + ScreenCenterX < 0) TargetX = 0;
				}
			}

			if (tary != 0)
			{
				if (tary > ScreenCenterY)
				{
					TargetY = -(ScreenCenterY - tary);
					TargetY /= smooth;
					if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
				}

				if (tary < ScreenCenterY)
				{
					TargetY = tary - ScreenCenterY;
					TargetY /= smooth;
					if (TargetY + ScreenCenterY < 0) TargetY = 0;
				}
			}
			mouse_event_((DWORD)MOUSEEVENTF_MOVE, (DWORD)TargetX, (DWORD)TargetY, (DWORD)0, (ULONG_PTR)0, autoshoot);
		}

		SHORT NtGetAsyncKeyState(int key)
		{
			static bool doneonce;
			if (!doneonce)
			{
				if (!GetSyscallIndex(_("win32u.dll"), _("NtUserGetAsyncKeyState"), _NtUserGetAsyncKeyState))
					return 0;
				doneonce = true;
			}
			return _NtUserGetAsyncKeyState(key);
		}
	}
}