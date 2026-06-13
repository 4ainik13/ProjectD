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
#include<cmath>

using namespace std;
using glm::vec2;

template <typename T>
class Latex
{
public:
	bool USE_SEED_COLUMN = true;
	bool USE_LAST_FRAME_COLUMN = true;

	queue<string> opening;
	stack<string> ending;
	vector<string> core;

	//Максимальное количество колонок в таблице.
	//Или максимальный размер ряда.
	int max_data_cols = 14;
	//Максимально количество рядов в таблице.
	//Или максимальный размер колонки.
	int max_data_rows = 10;

	//Данные, которые будут выведены в таблицу
	//data имеет размеры max_data_rows * max_data_cols.
	vector<vector<T>> data;

	vector<vector<string>> endingColumns;

	//Количество колонок
	//Вектор должен иметь размер max_data_cols.
	vector<int> biggestColNumber;
	//Последнее принятое значение в ряду. Не факт, что оно было записано в data
	//Вектор должен иметь размер max_data_cols.
	vector<T> lastValue;
	//Наименьшее принятое значение в ряду. Не факт, что оно было записано в data
	//Вектор должен иметь размер max_data_cols.
	vector<T> smallestValue;
	//Номер колонки наименьшего принятого значения в ряду. Не факт, что оно было записано в data
	//Вектор должен иметь размер max_data_cols.
	vector<int> colOfSmallestValue;

	//Закрывающая таблицу колонка. 
	//Состоит из значений, которые мы хотим добавить в конец каждого ряда.
	//Может состоять из пустых значений.
	//Вектор должен иметь размер max_data_rows.
	vector<string> endingColumn;

	//Открывающая таблицу колонка. 
	//Вектор должен иметь размер max_data_rows.
	vector <string> openingColumn;

	//Вектор должен иметь размер max_data_rows.
	vector <string> seedColumn;

	//Самый большой номер колонки среди всех рядов.
	//Или самый большой размер среди всех рядов.
	int biggestColNumberAcrossAllRows;

	bool endingColumnFilled;
	bool openingColumnFilled;
	bool seedColumnFilled;

	// !!! СДЕЛАТЬ ТАК, ЧТОБЫ ДЛЯ ОДИНАКОВЫХ СЛУЧАЕВ НЕ ВЫВОДИЛИСЬ ПОВТОРНАЯ СТРОКА СО СТОЛБИКАМИ
	// !!! НО КОГДА ИДЁТ РАЗРЫВ - ВЫВОДИЛОСЬ

	Latex()
	{
		biggestColNumberAcrossAllRows = 0;

		endingColumn = vector<string>(max_data_rows, "");
		openingColumn = vector<string>(max_data_rows, "");
		seedColumn = vector<string>(max_data_rows, "");
		endingColumnFilled = false;
		openingColumnFilled = false;
		seedColumnFilled = false;

		biggestColNumber = vector<int>(max_data_cols, 0);
		lastValue = vector<T>(max_data_cols, 0);
		smallestValue = vector<T>(max_data_cols, INT_MAX);
		colOfSmallestValue = vector<int>(max_data_cols, 0);
		data = vector<vector<T>>(max_data_rows, vector<T>(max_data_cols, -1));
	}

	Latex(int colSize, int rowSize) : Latex()
	{
		max_data_rows = colSize;
		max_data_cols = rowSize;
	}

	Latex(int colSize, int rowSize, bool useSeed, bool useLastFrame) : Latex(colSize, rowSize)
	{
		USE_SEED_COLUMN = useSeed;
		USE_LAST_FRAME_COLUMN = useLastFrame;
	}

	//row и col должны начинаться с 0
	void parse(const T& val, const int& row, const int& col)
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

	void parseVector(const vector<double>& inputVector, const int& row)
	{
		for (int i = 0; i < inputVector.size(); i++)
		{
			parse(inputVector[i], row, i);
		}
	}

	void parseSeed(const time_t& seed, const int& row)
	{
		if (row >= max_data_rows) return;
		if (!USE_SEED_COLUMN) return;
		seedColumn[row] = to_string(seed);
	}

	void writeTable(const vec2& a_vec, const vec2& b_vec, const float& prob, const string& fileName = "texTable.txt", const string & colType = "l", const bool& doBorder = true)
	{
		if(USE_LAST_FRAME_COLUMN) fillColumn(endingColumn, biggestColNumber);
		fillColumn(openingColumn, &openingColumnFilled);

		if (USE_LAST_FRAME_COLUMN) endingColumns.push_back(endingColumn);
		if (USE_SEED_COLUMN) endingColumns.push_back(seedColumn);

		char off = 0;
		char cou = 4;
		string capStr =
			"$\\vec{a} = (" + to_string(a_vec.x).substr(off, cou) + ", " + to_string(a_vec.y).substr(off, cou) + ")\\quad \\vec{b} = (" + to_string(b_vec.x).substr(off, cou) + ", " + to_string(b_vec.y).substr(off, cou) + ")\\quad P = " + to_string(prob).substr(off, cou) + "$";
		int numberOfCols = max_data_cols;
		numberOfCols += endingColumns.size();
		if (openingColumnFilled) numberOfCols++;

		beginTable();
		
		openingCaption(capStr);
		openingCentering();
		openingSetlenghtTabcolsep();
		beginTabular(numberOfCols, colType, doBorder);

		writeData(max_data_rows, max_data_cols, 1);

		writeFile(fileName);
	}

	void writeTabular(const string& fileName, const string& colType = "l", const bool& doBorder = true)
	{
		fillColumn(openingColumn, vector<string>{"FPS"}, & openingColumnFilled);

		int numberOfCols = max_data_cols;
		if (openingColumnFilled) numberOfCols++;

		beginTabular(numberOfCols, colType, doBorder);

		writeData(max_data_rows, max_data_cols, 1);

		writeFile(fileName);
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

	void openingSetlenghtTabcolsep(const string& value = "4pt")
	{
		writeOpening(buildSetlengthTabcolsep(value));
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

	void writeData(const int& colSize, const int& rowSize, int columnRowAmount = INT_MAX)
	{
		for (int i = 0; i < colSize; i++)
		{
			hline();
			if (columnRowAmount > 0)
			{
				writeColumnRow(i, rowSize);
				columnRowAmount--;
			}
			writeDataRow(i, rowSize);
		}
	}

	void writeColumnRow(const int& row, const int& rowSize)
	{
		writeCore(columnRowBuffer(row, rowSize), true);
	}

	void writeDataRow(const int& row, const int& rowSize)
	{
		writeCore(dataRowBuffer(row, rowSize), true);
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

	void fillColumn(vector<string>& columnToFill, const vector<int>& srcColumn, bool* flag_columnFilled = nullptr)
	{
		for (int i = 0; i < columnToFill.size() && i < srcColumn.size(); i++)
		{
			columnToFill[i] = to_string(srcColumn[i]);
		}
		if (flag_columnFilled) (*flag_columnFilled) = true;
	}

	void fillColumn(vector<string>& columnToFill, bool* flag_columnFilled = nullptr)
	{
		for (int i = 0; i < columnToFill.size(); i++)
		{
			columnToFill[i] = to_string(i + 1);
		}
		if (flag_columnFilled) (*flag_columnFilled) = true;
	}

	void fillColumn(vector<string>& columnToFill, const vector<string>& srcVector, bool* flag_columnFilled = nullptr)
	{
		for (int i = 0; i < columnToFill.size() && i < srcVector.size(); i++)
		{
			columnToFill[i] = srcVector[i];
		}
		if (flag_columnFilled) (*flag_columnFilled) = true;
	}

	string columnRowBuffer(const int& row, const int& rowSize)
	{
		string builder = "";
		string strArr[] = { "$N_i$", "Seed"};

		if (openingColumnFilled) builder.append("i & ");
		builder.append(buildColumnRow(row, rowSize));
		builder.append(buildPrefaceEndingColumns(strArr));

		builder.append(buildRowEnd());

		return builder;
	}

	string dataRowBuffer(const int& row, const int& rowSize)
	{
		string builder = "";

		if (openingColumnFilled) builder.append(openingColumn[row] + " & ");
		builder.append(buildDataRow(row, rowSize));
		builder.append(buildDataEndingColumns(row));

		builder.append(buildRowEnd());

		return builder;
	}

	string buildDataEndingColumns(const int& row)
	{
		string builder = "";

		for (int i = 0; i < endingColumns.size(); i++)
		{
			builder.append(" & ");
			builder.append(endingColumns[i][row]);
		}

		return builder;
	}

	string buildPrefaceEndingColumns(const string* strArr)
	{
		string builder = "";

		for (int i = 0; i < endingColumns.size(); i++)
		{
			builder.append(" & ");
			builder.append(strArr[i]);
		}

		return builder;
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

		return builder;
	}

	string buildSlicedColumnRow(const int& rowSize, const int& lastColNumber)
	{
		string builder = "";

		builder.append(buildColumnRowBasis(rowSize - 2));
		builder.append(" & ... & ");
		//builder.append(to_string(lastColNumber));
		builder.append("N");

		return builder;
	}

	string buildColumnRowWithMin(const int& rowSize, const int& lastColNumber, const int& lastColRow)
	{
		string builder = "";

		builder.append(buildColumnRowBasis(rowSize - 4));
		builder.append(" & ... & ");
		//builder.append(to_string(colOfSmallestValue[lastColRow]));
		builder.append("Min");
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

		string builder = "";
		if (lastColNumber > 0) builder.append(" & ");

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
			builder.append(toString(data[row][i]));
			if (i + 1 < rowSize)builder.append(" & ");
		}

		return builder;
	}

	string fillEmptyDataRow(const int& row, const int& rowSize, const int& lastColNumber)
	{
		if (rowOutOfBounds(row)) return "";
		if (lastColNumber >= rowSize) return "";

		string builder = "";
		if (lastColNumber > 0) builder.append(" & ");

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


	string buildSetlengthTabcolsep(const string& value)
	{
		return buildSetlenght(buildTabcolsep(), value);
	}

	string buildCaption(const string& captionStr)
	{
		return "\\caption{" + captionStr + "}";
	}

	string buildCentering()
	{
		return "\\centering";
	}

	string buildTabcolsep()
	{
		return "\\tabcolsep";
	}

	string buildSetlenght(const string& param, const string& value)
	{
		return "\\setlength" + param + "{" + value + "}";
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

	string toString(const int& val, const int& precision = 2)
	{
		return to_string(val);
	}

	string toString(const double& val, const int& precision = 2)
	{
		string builder = to_string(val);
		int dot_pos = builder.find(".");
		if (dot_pos == string::npos) return builder;
		if (builder.length() - 1 - dot_pos > precision)
		{
			float volatileNumber = stof(builder.substr(0, dot_pos + precision+1));
			int decidingCipher = builder[dot_pos + precision + 1] - '0';
			if (decidingCipher >= 5) volatileNumber + 1/std::pow(10.f,precision);
			builder = to_string(volatileNumber);
			dot_pos = builder.find(".");
			builder = builder.substr(0, dot_pos + precision + 1);
		}
		return builder;
	}
};
