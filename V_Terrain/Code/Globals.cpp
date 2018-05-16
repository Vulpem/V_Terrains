#include "Globals.h"
#include <cstdarg>
#include "Terrainconfig.h"
#include <assert.h>

namespace VTerrain
{
	namespace utils
	{
		int FormatCString(char *outBuffer, int outBufferSize, const char* format, va_list args)
		{
			if (!(outBuffer != nullptr && format != nullptr)) { DebugBreak(); }

			static constexpr char endOfLineChars[]{ '\r', '\n', '\0' };

			char* p = outBuffer;
			const int n = _vsnprintf_s(outBuffer, outBufferSize - sizeof(endOfLineChars) + 1, outBufferSize - sizeof(endOfLineChars), format, args);

			if (n < 0)
				return n;

			p += static_cast<size_t>(n);

			for (const auto& character : endOfLineChars)
				*p++ = character;

			return n;
		}

		void ShowAlertPopup(const char * file, const char * function, const int line, const char * format, ...)
		{
			va_list args;
			va_start(args, format);
			char buffer[4096];
			FormatCString(buffer, format, args);
			va_end(args);

			char message[4096];

			sprintf_s(message, "File: %s\nFunction: %s\nLine: %-5d\n\n%s",
				file, function,
				line, buffer);

			config.throwErrorFunc(message);
		}
	}
}