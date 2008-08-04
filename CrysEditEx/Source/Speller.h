#pragma once

#include <fstream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <iterator>
#include <cstdlib>
#include <memory>

#include "hunspell.hxx"

class Speller
{
	std::auto_ptr<Hunspell> spell_;

	// No copying
	Speller(const Speller&);
	Speller& operator=(const Speller&);

	typedef std::basic_string<TCHAR> StringType;
	typedef std::tr1::unordered_set<StringType> WordContainer;
	WordContainer ignored_words_;
	bool added_modified_, ignored_modified_;

	WordContainer added_words_;

public:
	Speller(LPCTSTR affpath, LPCTSTR dpath, LPCTSTR key = 0)
			: added_modified_(false)
			, ignored_modified_(false)
	{
		USES_CONVERSION;
		spell_.reset(new Hunspell(T2CA(affpath),T2CA(dpath),T2CA(key)));
	}

	//~Speller()
	//{
	//}

	bool is_added_modified() const { return added_modified_; }

	bool is_ignored_modified() const { return ignored_modified_; }

	int add(LPCTSTR w)
	{
		if (added_words_.insert(w).second)
			added_modified_ = true;

		USES_CONVERSION;
		return spell_->add(T2CA(w));
	}

	void add_dic(LPCTSTR dpath, LPCTSTR key = 0)
	{
		USES_CONVERSION;
		spell_->add_dic(T2CA(dpath),T2CA(key));
	}

	int remove(LPCTSTR word)
	{
		USES_CONVERSION;
		return spell_->remove(T2CA(word));
	}

	void ignore(LPCTSTR word)
	{
		if (ignored_words_.insert(word).second)
			ignored_modified_ = true;
	}

	bool spell(LPCTSTR word)
	{
		USES_CONVERSION;
		bool result = spell_->spell(T2CA(word)) != 0;

		if (!result)
			result = added_words_.find(word) != added_words_.end() ||
			         ignored_words_.find(word) != ignored_words_.end();

		return result;
	}

	int suggest(LPCTSTR word, CStringArray& a)
	{
		USES_CONVERSION;
		a.RemoveAll();

		char** l;
		int count = spell_->suggest(&l,T2CA(word));

		if (count > 0)
		{
			for (int i = 0; i < count; ++i)
			{
				a.Add(A2CT(l[i]));
				std::free(l[i]);
			}

			std::free(l);
		}

		return count;
	}

	void open_ignored_words(LPCTSTR path)
	{
		std::basic_ifstream<TCHAR> in(path);
		typedef std::istream_iterator<StringType,StringType::value_type,StringType::traits_type> iterator;

		if (in)
			std::copy(iterator(in),iterator(),std::inserter(ignored_words_,ignored_words_.end()));
	}

	void save_ignored_words(LPCTSTR path)
	{
		std::basic_ofstream<TCHAR> out(path);
		typedef std::ostream_iterator<StringType,StringType::value_type,StringType::traits_type> iterator;

		if (out)
		{
			std::copy(ignored_words_.begin(),ignored_words_.end(),
			          iterator(out,_T("\n")));
			ignored_modified_ = false;
		}
	}

private:
	void xadd(const StringType& s)
	{
		USES_CONVERSION;
		spell_->add(T2CA(s.c_str()));
	}

public:
	void open_personal_dictionary(LPCTSTR path)
	{
		std::basic_ifstream<TCHAR> in(path);
		typedef std::istream_iterator<StringType,StringType::value_type,StringType::traits_type> iterator;

		if (in)
		{
			std::copy(iterator(in),iterator(),
			          std::inserter(added_words_,added_words_.end()));

			using std::tr1::bind;
			using namespace std::tr1::placeholders;

			std::for_each(added_words_.begin(),added_words_.end(),
			              bind(&Speller::xadd,this,_1));
		}
	}

	void save_personal_dictionary(LPCTSTR path)
	{
		std::basic_ofstream<TCHAR> out(path);
		typedef std::ostream_iterator<StringType,StringType::value_type,StringType::traits_type> iterator;

		if (out)
		{
			std::copy(added_words_.begin(),added_words_.end(),
			          iterator(out,_T("\n")));
			added_modified_ = false;
		}
	}
};