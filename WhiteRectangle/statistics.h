#pragma once
#include<vector>

using namespace std;

class Statistics
{
public:

	//Задаём начальное количество рядов
	void initRows(const int& numberOfRows = 10)
	{
		data = vector<vector<double>>(numberOfRows % max_rows);
	}

	//Заносим данные в матрицу. Данные val заносятся в столбец col ряда row
	void parse(const int& row, const int& col, const double& val)
	{
		createRow(row);
		createCol(row, col);
		data[row][col] = val;
	}

	//Возвращает ссылку на вектор со средними значениями по всем рядам. 
	//Возвращаемый вектор имеет размер data.size() - количество рядов в матрице data.
	vector<double> average()
	{
		vector<double> avrg(data.size());
		double buffer;

		for (int i = 0; i < data.size(); i++)
		{
			buffer = 0;
			for (int j = 0; j < data[i].size(); j++)
			{
				buffer += data[i][j];
			}
			avrg[i] = buffer / data[i].size();
		}

		return avrg;
	}

private:
	const int max_rows = 50;
	const int max_cols = 10000;
	
	vector<vector<double>> data;

	void createRow(const int& row)
	{
		if (rowExists(row) || badRow(row)) return;
		for (int i = data.size(); i <= row; i++)
		{
			data.push_back(vector<double>());
		}
	}

	void createCol(const int& row, const int& col)
	{
		if (colExists(row, col) || badCol(col)) return;
		for (int i = data[row].size(); i <= col; i++)
		{
			data[row].push_back(0);
		}
	}

	bool rowExists(const int& row)
	{
		if (row < data.size()) return true;
		return false;
	}

	bool badRow(const int& row)
	{
		if (row < 0) return true;
		if (row >= max_rows) return true;
		return false;
	}

	bool colExists(const int& row, const int& col)
	{
		if (badRow(row)) return true;
		if (col < data[row].size()) return true;
		return false;
	}

	bool badCol(const int& col)
	{
		if (col < 0) return true;
		if (col >= max_cols) return true;
		return false;
	}
};
