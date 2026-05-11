#pragma once
#include <glm/glm.hpp>

#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <iostream>

using namespace std;
using glm::vec2;

class Latex
{
public:
	queue<string> opening;
	stack<string> ending;
	vector<string> core;

	//rowSize
	const int max_data_cols = 14;
	//colSize
	const int max_data_rows = 10;

	vector<vector<int>> data;
	vector<int> biggestColNumber;
	vector<int> lastValue;
	vector<int> smallestValue;
	vector<int> colOfSmallestValue;

	int biggestColNumberAcrossAllRows;

	Latex()
	{
		biggestColNumberAcrossAllRows = 0;

		biggestColNumber = vector<int>(max_data_cols, 0);
		lastValue = vector<int>(max_data_cols, 0);
		smallestValue = vector<int>(max_data_cols, INT_MAX);
		colOfSmallestValue = vector<int>(max_data_cols, 0);
		data = vector<vector<int>>(max_data_rows, vector<int>(max_data_cols, -1));
	}

	//col Должен начинаться с 0
	void parse(const int& val, const int& row, const int& col)
	{
		if (row >= max_data_rows) return;

		lastValue[row] = val;

		if (val < smallestValue[row])
		{
			smallestValue[row] = val;
			colOfSmallestValue[row] = col;
		}

		if (col+1 > biggestColNumber[row])
		{
			biggestColNumber[row] = col+1;
			if (col+1 > biggestColNumberAcrossAllRows)
				biggestColNumberAcrossAllRows = col+1;
		}

		if (col < max_data_cols) 
			data[row][col] = val;
	}

	void writeTable(const vec2& a_vec, const vec2& b_vec, const float& prob, const string& colType = "l", const bool& doBorder = true)
	{
		beginTable();

		char off = 0;
		char cou = 4;
		string capStr =
			"$\\vec{a} = ("+ to_string(a_vec.x).substr(off, cou) + ", " + to_string(a_vec.y).substr(off, cou) + ")\\quad \\vec{b} = (" + to_string(b_vec.x).substr(off, cou) + ", " + to_string(b_vec.y).substr(off, cou) + ")\\quad P = " + to_string(prob).substr(off, cou) + "$";
		
		openingCaption(capStr);
		openingCentering();
		beginTabular(max_data_cols, colType, doBorder);

		for (int i = 0; i < max_data_rows; i++)
		{
			hline();
			writeColumnRow(i, max_data_cols);
			writeDataRow(i, max_data_cols);
		}

		writeFile();
	}

private:

	void hline(const string& addendum = "", const bool& doBorder = true)
	{
		writeCore(buildHline(addendum, doBorder), true);
	}

	void centering()
	{
		writeCore(buildCentering(), true);
	}

	void caption(const string& captionStr)
	{
		writeCore(buildCaption(captionStr), true);
	}

	void openingCentering()
	{
		writeOpening(buildCentering());
	}
	
	void openingCaption(const string& captionStr)
	{
		writeOpening(buildCaption(captionStr));
	}

	void beginTabular(const int& numberOfCols, const string& colType = "l", const bool doBorder = true)
	{
		cap("tabular", "{" + buildColumnType(numberOfCols, colType, doBorder) + "}");
	}

	void beginTable()
	{
		cap("table", "[!h]");
	}

	void writeColumnRow(const int& row, const int& rowSize)
	{
		writeCore(buildColumnRow(row, rowSize), true);
	}

	void writeDataRow(const int& row, const int& rowSize)
	{
		writeCore(buildDataRow(row, rowSize), true);
	}

	void writeFile(const string& fileName = "texTable.txt")
	{
		std::ofstream out;
		out.open(fileName);

		if (out.is_open())
		{
			while (!opening.empty())
			{
				out << opening.front();
				opening.pop();
			}

			for (int i = 0; i < core.size(); i++)
			{
				out << core[i];
			}

			while (!ending.empty())
			{
				out << ending.top();
				ending.pop();
			}

			std::cout << "Table file created\n";
		}
		else
			std::cout << "Couldn't open table file\n";

		out.close();
	}

	string buildColumnRow(const int& row, const int& rowSize)
	{
		string builder = "";
		int lastColNumber = biggestColNumber[row];

		if (lastColNumber <= max_data_cols)
		{
			builder.append(buildSmallColumnRow(rowSize, lastColNumber));
		}
		else if (lastValue[row] > 1)
		{
			builder.append(buildColumnRowWithMin(rowSize, lastColNumber, row));
		}
		else
		{
			builder.append(buildSlicedColumnRow(rowSize, lastColNumber));
		}

		builder.append(buildRowEnd());

		return builder;
	}

	string buildSlicedColumnRow(const int& rowSize, const int& lastColNumber)
	{
		string builder = "";

		builder.append(buildColumnRowBasis(rowSize - 2));
		builder.append(" & ... & ");
		builder.append(to_string(lastColNumber));

		return builder;
	}

	string buildColumnRowWithMin(const int& rowSize, const int& lastColNumber, const int& lastColRow)
	{
		string builder = "";

		builder.append(buildColumnRowBasis(rowSize - 4));
		builder.append(" & ... & ");
		builder.append(to_string(colOfSmallestValue[lastColRow]));
		builder.append(" & ... & ");
		builder.append(to_string(lastColNumber));

		return builder;
	}

	string buildSmallColumnRow(const int& rowSize, const int& lastColNumber)
	{
		string builder = "";

		builder.append(buildColumnRowBasis(lastColNumber));
		builder.append(fillEmptyColumnRow(rowSize, lastColNumber));

		return builder;
	}

	string buildDataRow(const int& row, const int& rowSize)
	{
		if (rowOutOfBounds(row)) return "";

		string builder = "";
		int lastColNumber = biggestColNumber[row];

		if (lastColNumber <= max_data_cols)
		{
			builder.append(buildSmallDataRow(row, rowSize, lastColNumber));
		}
		else if (lastValue[row] > 1)
		{
			builder.append(buildDataRowWithMin(row, rowSize)) ;
		}
		else
		{
			builder.append(buildSlicedDataRow(row, rowSize)) ;
		}

		builder.append(buildRowEnd());

		return builder;
	}

	string buildSlicedDataRow(const int& row, const int& rowSize)
	{
		if (rowOutOfBounds(row)) return "";

		string builder = "";

		builder.append(buildDataRowBasis(row, rowSize-2));
		builder.append(" & ... & ");
		builder.append(to_string(lastValue[row]));

		return builder;
	}

	string buildDataRowWithMin(const int& row, const int& rowSize)
	{
		if (rowOutOfBounds(row)) return "";

		string builder = "";

		builder.append(buildDataRowBasis(row, rowSize - 4));
		builder.append(" & ... & ");
		builder.append(to_string(smallestValue[row]));
		builder.append(" & ... & ");
		builder.append(to_string(lastValue[row]));

		return builder;
	}

	string buildSmallDataRow(const int& row, const int& rowSize, const int& lastColNumber)
	{
		if (rowOutOfBounds(row)) return "";

		string builder = "";

		builder.append(buildDataRowBasis(row, lastColNumber));
		builder.append(fillEmptyDataRow(row, rowSize, lastColNumber));

		return builder;
	}

	string buildColumnRowBasis(const int& lastColNumber)
	{
		string builder = "";

		for (int i = 1; i < lastColNumber + 1; i++)
		{
			builder.append(to_string(i));
			if (i + 1 < lastColNumber + 1)builder.append(" & ");
		}

		return builder;
	}

	string fillEmptyColumnRow(const int& rowSize, const int& lastColNumber)
	{
		if (lastColNumber + 1 >= rowSize + 1) return "";

		string builder = " & ";

		for (int i = lastColNumber + 1; i < rowSize + 1; i++)
		{
			builder.append(to_string(i));
			if (i + 1 < rowSize + 1)builder.append(" & ");
		}

		return builder;
	}

	string buildDataRowBasis(const int& row, const int& rowSize)
	{
		if (rowOutOfBounds(row)) return "";

		string builder = "";

		for (int i = 0; i < rowSize; i++)
		{
			builder.append(to_string(data[row][i]));
			if (i + 1 < rowSize)builder.append(" & ");
		}

		return builder;
	}

	string fillEmptyDataRow(const int& row, const int& rowSize, const int& lastColNumber)
	{
		if (rowOutOfBounds(row)) return "";
		if (lastColNumber >= rowSize) return "";

		string builder = " & ";

		for (int i = lastColNumber; i < rowSize; i++)
		{
			builder.append(" ");
			if (i + 1 < rowSize)builder.append(" & ");
		}

		return builder;
	}

	string buildColumnType(const int& numberOfCols, const string& type = "l", const bool& doBorder = true)
	{
		if (numberOfCols <= 0) return "";

		string builder = "";
		string delim = "";
		if (doBorder) delim = "|";

		builder.append(delim);
		for (int i = 0; i < numberOfCols; i++)
		{
			builder.append(type);
			builder.append(delim);
		}

		return builder;
	}

	string buildRowEnd()
	{
		return buildHline(" \\\\ ");
	}

	string buildHline(const string& addendum = "", const bool& doBorder = true)
	{
		if (doBorder)
			return addendum + "\\hline";
		else
			return addendum;
	}

	string buildCaption(const string& captionStr)
	{
		return "\\caption{" + captionStr + "}";
	}

	string buildCentering()
	{
		return "\\centering";
	}

	bool rowOutOfBounds(const int& row)
	{
		return row >= max_data_rows;
	}

	void cap(const string& toCapWith, const string& addendum = "")
	{
		begin(toCapWith, addendum);
		end(toCapWith);
	}

//	void begin(const string& beginWith, const string& attribute = "")
//	{
//		writeOpening(format("\\begin{%s}%s", beginWith, attribute));
//	}

	void begin(const string& beginWith, const string& attribute = "")
	{
		writeOpening("\\begin{" + beginWith + "}" + attribute);
	}

	void end(const string& endWith)
	{
		writeEnding("\\end{" + endWith + "}");
	}

	void writeCore(const string& toWrite, const bool& endLine = false)
	{
		if(endLine)
			core.push_back(toWrite + "\n");
		else
			core.push_back(toWrite + " ");
	}

	void writeOpening(const string& toWrite, const bool& endLine = true)
	{
		writeQueue(opening, toWrite, endLine);
	}

	void writeEnding(const string& toWrite, const bool& endLine = true)
	{
		writeStack(ending, toWrite, endLine);
	}

	void writeQueue(queue<string>& q, const string& toWrite, const bool& endLine = false)
	{
		if (endLine)
			q.push(toWrite + "\n");
		else
			q.push(toWrite + " ");
	}

	void writeStack(stack<string>& stk, const string& toWrite, const bool& endLine = false)
	{
		if (endLine)
			stk.push(toWrite + "\n");
		else
			stk.push(toWrite + " ");
	}
};