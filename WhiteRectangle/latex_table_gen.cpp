#include <fstream>
#include <vector>

using std::vector;



namespace latex
{
	const int tableValuesSize = 18;
	vector<int> tableValues;

	void genTable()
	{
		std::ofstream out;
		out.open("table.txt");
		if (out.is_open())
		{
			out << "\\begin{table}[h]\n";
			out << "\\begin{tabular}{" << genRows() << "}\n";
			out << "\\hline\n";

		}
		out.close();
	}

	namespace
	{
		std::string genRows()
		{
			std::string builder = "|l|";
			for (int i = 0; i < tableValuesSize; i++)
			{
				builder.append("l|");
			}
			return builder;
		}

		std::string genFrames()
		{

		}
	}
}