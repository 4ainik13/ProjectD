#pragma once

namespace dbg
{
	void printGlError(GLenum GlError, std::string context = "DEBUG");
	void printAllGlErrors(std::string context = "DEBUG");
}
