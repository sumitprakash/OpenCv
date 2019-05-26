#include <conio.h>
#include <cctype>
#include <regex>
#include <string>

#include "ocr.h"
#include "utility.h"
#include "GenerateTrainingModel.h"

typedef vector<string> strVector;
typedef map<string, vector<int>> templateFeature;

struct lineItemDetails
{
	int lineNumber;
	int quality;
	string description;
	float unitAmount;
	float totalLineAmunt;
};

map<int, map<string, strVector>> lineHeaders;
map<int, string> headerOfInvoice;
vector<vector<int>> headerBoundary;

void InitializeLineHeader()
{
	lineHeaders[0]["quantity"] = { "QUANTITY" };
	lineHeaders[1]["description"] = { "DESCRIPTION" };
	lineHeaders[2]["lineAmount"] = { "AMOUNT" };
	lineHeaders[3]["unitPrice"] = { "PRICE" };
	lineHeaders[4]["lineNumber"] = { "PO LINE NUMBER" };
	lineHeaders[5]["vat"] = { "VAT" };
}

void IdentifyHeaderBoundary()
{
	bool flag = true;
	vector<int> boundary;

	for (auto temp : headerOfInvoice)
	{
		if (flag == false)
		{
			boundary.push_back(temp.first);
			flag = true;
		}
		else
		{
			boundary.push_back(temp.first - 1);
			headerBoundary.push_back(boundary);
			boundary.clear();
			boundary.push_back(temp.first);
		}
		boundary.push_back(64000);
		headerBoundary.push_back(boundary);
	}
}

void OptimalHeaderMatching(map<int, map<int, string>> &result)
{

}

bool MatchLineHeaders(map<int, string> line)
{
	int noOfSpace = 0;
	int noOfMatchedHeaders = 0;
	bool headerMatch = false;

	string headerStringFromResult = "";

	for (int i = 0; i < lineHeaders.size(); i++)
	{
		for (auto headerType : lineHeaders[i])
		{
			for (int pos = 0; pos < headerType.second.size(); pos++)
			{
				noOfSpace = 0;
				for (int i = 0; i < headerType.second[pos].size(); i++)
				{
					if (headerType.second[pos].at(i) == ' ')
					{
						noOfSpace++;
					}
				}
				auto tokens = line.begin();
				for (; tokens != line.end();)
				{
					headerStringFromResult = tokens->second;
					if (noOfSpace > 0)
					{
						auto tempItr = tokens;
						for (int i = noOfSpace; i != 0; i--)
						{
							tempItr++;
							if (tempItr != line.end())
							{
								headerStringFromResult = headerStringFromResult + " " + tempItr->second;
							}
							else
								break;
						}
					}
					if (StringCompare(headerType.second[pos], headerStringFromResult))
					{
						noOfMatchedHeaders++;
						headerOfInvoice[tokens->first] = headerType.first;
						headerMatch = true;
						break;
					}
					tokens++;
				}
				if (headerMatch)
				{
					headerMatch = false;
					break;
				}
			}
		}
	}
	if (noOfMatchedHeaders >= 2)
		return true;
	else
		return false;
}

bool FindLineHeaderValues(map<int, string> line, map<string, string> &headedrValues)
{
	auto headerItr = headerOfInvoice.begin();
	auto lineItr = line.begin();
	for (int i = 0; lineItr != line.end() && headerItr != headerOfInvoice.end() && i < headerBoundary.size();)
	{
		if (lineItr->first >= headerBoundary[i][0] - 5 && lineItr->first < headerBoundary[i][i])
		{
			if (headedrValues.find(headerItr->second) == headedrValues.end())
				headedrValues[headerItr->second] = lineItr->second;
			else
				headedrValues[headerItr->second] = headedrValues[headerItr->second] + " " + lineItr->second;
		}
		else
		{
			i++;
			headerItr++;
			if (lineItr != line.begin())
				lineItr--;
		}
		lineItr++;
	}
	if (headedrValues.size() == headerOfInvoice.size())
		return true;
	else
		return false;
}

void IdentifyLineItemHeader(map<int, map<int, string>> result)
{
	bool next = false;
	bool down = false;
	int x, y;
	vector<string> res;
	string headerStringFromResult = "";
	bool lineHeaderMatched = false;
	map<string, string> headerValues;
	vector<map<string, string>> lineItemValus;

	for (auto &row : result)
	{
		if (MatchLineHeaders(row.second) && !lineHeaderMatched)
		{
			IdentifyHeaderBoundary();
			lineHeaderMatched = true;
		}
		else if (lineHeaderMatched)
		{
			if (FindLineHeaderValues(row.second, headerValues))
			{
				lineItemValus.push_back(headerValues);
				headerValues.clear();
			}
			else
				break;
		}
	}
	for (auto lineItem : lineItemValus)
	{
		for (auto val : lineItem)
		{
			std::cout << val.first << ":" << val.second << ";";
		}
	}
}

void Refine(map<int, map<int, string>> &result)
{
	regex exp1("[A-Za-z]+0[A-Za-z]+", regex_constants::ECMAScript | regex_constants::icase);
	regex exp2("0[A-Za-z]{2,}", regex_constants::ECMAScript | regex_constants::icase);

	for (auto &temp1 : result)
	{
		for (auto &temp2 : temp1.second)
		{
			if (regex_match(temp2.second, exp2))
			{
				temp2.second.at(0) = 'O';
			}
			if (regex_match(temp2.second, exp1))
			{
				for (int i = 1; i < temp2.second.size() - 1; i++)
				{
					if (temp2.second.at(i) == '0')
						temp2.second.at(i) = 'O';
				}
			}
			if (temp2.second == "T0")
				temp2.second = "TO";
			else if (temp2.second == "t0")
				temp2.second = "to";
		}
	}
}

bool SearchDate(string str)
{
	try
	{
		regex exp1("\\d{1,2}[/ :]\\d{1,2}[/ :]\\d{2,4}", std::regex_constants::ECMAScript | regex_constants::icase);
		if (regex_match(str, exp1))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (regex_error e)
	{
		cout << e.what();
		_getch();
		return false;
	}
}

string SearchHeader(string header, vector<string> subHeader, map<int, map<int, string>> &result)
{
	bool next = false;
	bool down = false; int x, y;
	vector<string> res;
	string headerStringFromResult = "";
	int subHeaderSize = subHeader.size();
	int noOfSpace = 0;

	for (int i = 0; i < header.size(); i++)
	{
		if (header.at(i) == ' ')
			noOfSpace++;
	}

	for (auto &temp1 : result)
	{
		auto temp2 = temp1.second.begin();
		for (; temp2 != temp1.second.end();)
		{
			headerStringFromResult = temp2->second;
			if (noOfSpace > 0)
			{
				auto tempItr = temp2;
				for (int i = noOfSpace; i != 0; i--)
				{
					tempItr++;
					if (tempItr != temp1.second.end())
					{
						headerStringFromResult = headerStringFromResult + " " + tempItr->second;
					}
					else
						break;
				}
			}
			if (StringCompare(header, headerStringFromResult))
			{
				x = temp2->first;
				y = temp1.first;
				next = true;
			}
			else if(next)
			{
				bool flag = 0;
				for (int i = 0; i < subHeaderSize; i++)
				{
					if (StringCompare(subHeader[i], temp2->second))
					{
						flag = 1;
						break;
					}
				}
				if (flag == 0)
				{
					res.push_back(temp2->second);
					next = false;
					break;
				}
			}
			else if (down)
			{
				if (x >= temp2->first - 20 && x <= temp2->first + 20)
				{
					res.push_back(temp2->second);
					down = false;
					break;
				}
			}
			temp2++;
		}
		if (down)
			down = false;
		if (next)
		{
			down = true;
			next = false;
		}
	}
	if (res.size() > 0)
		return res.back();
	return "";
}

void InvoiceProcessoor(string imageFileName)
{
	map<int, map<int, string>> result;
	Predict(imageFileName, result, true);

	Refine(result);

	for (auto temp1 : result)
	{
		for (auto temp2 : temp1.second)
		{
			cout << temp2.second << " ";
		}
		cout << endl;
	}
	cout << endl;
	for (auto temp1 : result)
	{
		for (auto temp2 : temp1.second)
		{
			if (SearchDate(temp2.second))
			{
				cout << "Date:" << temp2.second << endl;
			}
		}
	}

	for (auto temp1 : result)
	{
		cout << "VendorName:";

		string vendor;
		for (auto temp2 : temp1.second)
		{
			vendor += temp2.second;
			vendor += " ";
		}
		vendor = vendor.substr(0, vendor.length() - 1);
		cout << vendor << endl;
		break;
	}

	vector<string> subHeader;
	subHeader.push_back("number");
	subHeader.push_back("no.");
	cout << "Invoice:" << SearchHeader("invoice", subHeader, result) << endl;

	subHeader.clear();
	subHeader.push_back("order");
	cout << "PO:" << SearchHeader("purchase order", subHeader, result) << endl;

	subHeader.clear();
	cout << "Total:" << SearchHeader("total", subHeader, result) << endl;

	cout << "SubTotal:" << SearchHeader("subTotal", subHeader, result) << endl;
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		if (IsWordsEqual(argv[1], "generate"))
		{
			GenerateTrainingMoodel();
		}
	}
	else if (argc == 3)
	{
		if (IsWordsEqual(argv[1], "predict"))
		{
			try
			{
				InvoiceProcessoor(argv[2]);
			}
			catch (const std::exception ex)
			{
				cout << ex.what(); 
				_getch();
				return 0;
			}
		}
	}
	return _getch();
}