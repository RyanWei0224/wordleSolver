#include <iostream>
#include <fstream>
#include <string.h>
#include <thread>
//#include <windows.h>
#include "words.h"

#define LEN 5

#define NTHREAD 7

using namespace std;

struct word{
	char alph[LEN];
	friend ostream& operator<< (ostream& out, const word& w){
		for(int i=0;i<LEN;++i){
			out<<w.alph[i];
		}
		return out;
	}
};

#if LEN <= 5
typedef uint8_t colors;
static constexpr colors cbit[6] = {1,3,9,27,81,243};
#elif LEN <= 10
typedef uint16_t colors;
static constexpr colors cbit[11] = {1,3,9,27,81,243,729,2187,6561,19683,59049};
#elif LEN <= 20
typedef uint32_t colors;
static constexpr colors cbit[21] = {1,3,9,27,81,243,729,2187,
	6561,19683,59049,177147,531441,1594323,4782969,14348907,
	43046721,129140163,387420489,1162261467,3486784401};
#else
#error "LEN not defined"
#endif

static constexpr size_t N = cbit[LEN];

//constexpr colors right_color = cbit[0] + cbit[1] + cbit[2] + cbit[3] + cbit[4];

static size_t cnum_all = 0;
static word* wordlist = nullptr;

//static size_t cnum = 0;
static word* cwlist = nullptr;
//static int* key = nullptr;

static bool hardMode = false;

void end(){
	delete[] wordlist;
	delete[] cwlist;
	//delete[] key;
}
#ifdef _WORDS_H
void init(){
	cnum_all = 0;
	for(size_t i = 0; i < sizeof(WORDS)/sizeof(const char*); ++i){
		if(strlen(WORDS[i]) == LEN){
			++cnum_all;
		}
	}
	cout<<cnum_all<<endl;
	wordlist = new word[cnum_all];
	cwlist = new word[cnum_all];
	size_t i = 0;
	for(size_t j = 0; j < cnum_all;++i){
		if(strlen(WORDS[i]) == LEN){
			memcpy(wordlist[j++].alph, WORDS[i], LEN);
		}
	}
}
#else
void init(){
	ifstream in("words.txt");
	in.seekg(0, in.end);
	long long fsize = in.tellg(), cpos = 0;
	in.seekg(0, in.beg);
	char* chs = new char[fsize];
	in.read(chs, fsize);
	bool isword = false;
	size_t i = 0;
	cnum_all = 0;
	while(cpos < fsize){
		if(chs[cpos++] == '"'){
			if(isword && i == LEN){
				++cnum_all;
			}
			isword = !isword;
			i = 0;
		}else if(isword){
			++i;
		}
	}
	cout<<cnum_all<<endl;
	wordlist = new word[cnum_all];
	cwlist = new word[cnum_all];
	//key = new int[cnum_all];
	cpos = 0;
	i = 0;
	while(i<cnum_all){
		while(chs[cpos++] != '"');
		char* c = wordlist[i].alph;
		int j;
		for(j=0;j<LEN;++j){
			c[j] = chs[cpos++];
			if(c[j]=='"'){
				break;
			}
		}
		if(j<LEN){
			continue;
		}
		if(chs[cpos++] == '"'){
			++i;
		}else{
			while(chs[cpos++] != '"');
		}
	}
}
#endif

colors get_color(const word& choose, const word& real){
	colors c = 0;
	bool color_x[LEN];
	memset(color_x,0,sizeof(bool)*LEN);
	for(unsigned i = 0; i < LEN; ++i){
		if(choose.alph[i] == real.alph[i]){
			c += cbit[i];
			color_x[i] = true;
		}
	}
	for(unsigned i = 0; i < LEN; ++i){
		if(choose.alph[i] != real.alph[i]){
			for(unsigned j = 0; j < LEN; ++j){
				if((! color_x[j]) && choose.alph[i] == real.alph[j]){
					color_x[j] = true;
					c += cbit[i] * 2;
					break;
				}
			}
		}
	}
	return c;
}

size_t compress(word guess, colors c, word* wlist, const size_t cnum){
	size_t new_num = 0;
	for(size_t i = 0; i < cnum; ++i){
		if(get_color(guess, wlist[i]) == c){
			cwlist[new_num++] = wlist[i];
		}
	}
	return new_num;
}

void _find_word_mth(const word* wlist, const size_t cnum,
					 const size_t from, const size_t to,
					 size_t* _cur_max, size_t* _cur_word){
	size_t keys[N];
	size_t& cur_max = *_cur_max;
	size_t& cur_word = *_cur_word;
	cur_max = cnum;
	cur_word = cnum_all;
	for(size_t i = from; i < to; ++i){
		memset(keys, 0, sizeof(keys));
		for(size_t j = 0; j < cnum; ++j) {
			colors c = get_color(wordlist[i], wlist[j]);
			++keys[c];
		}
		size_t maxnum = 0;
		for(size_t j = 0; j < N; ++j){
			maxnum = (maxnum > keys[j])? maxnum:keys[j];
		}
		if(maxnum < cur_max){
			cur_max = maxnum;
			cur_word = i;
		}
	}
}

size_t find_word_multi(const word* wlist, const size_t cnum, bool print = true, size_t cur_max = cnum_all){
	size_t cur_maxs[NTHREAD];
	size_t cur_words[NTHREAD];
	size_t from = 0, to = 0;
	thread t[NTHREAD];
	for(size_t i=0;i<NTHREAD;++i){
		from = to;
		to = ((i+1)*cnum_all+NTHREAD/2) / NTHREAD;
		t[i]=thread(_find_word_mth,wlist,cnum,from,to,cur_maxs+i,cur_words+i);
	}
	size_t cur_word = cnum_all;
	for(int i=0;i<NTHREAD;++i){
		t[i].join();
		if(cur_maxs[i] < cur_max){
			cur_max = cur_maxs[i];
			cur_word = cur_words[i];
		}
	}
	if(print)
		cout<<cur_max<<'/'<<cnum_all<<endl;
	return cur_word;
}

size_t find_word(const word* wlist, const size_t cnum, bool print = true){
	size_t cur_max = cnum;
	size_t cur_word = 0;
	size_t keys[N];
	for(size_t i = 0; i < cnum; ++i){
		memset(keys, 0, sizeof(keys));
		for(size_t j = 0; j < cnum; ++j) {
			colors c = get_color(wlist[i], wlist[j]);
			++keys[c];
		}
		size_t maxnum = 0;
		for(size_t j = 0; j < N; ++j){
			maxnum = (maxnum > keys[j])? maxnum:keys[j];
		}
		if(maxnum < cur_max){
			cur_max = maxnum;
			cur_word = i;
		}
	}
	if(wordlist != wlist && !hardMode){
		size_t new_word = find_word_multi(wlist, cnum, false, cur_max);
		if(new_word < cnum_all) cur_word = cnum_all + new_word;
	}
	if(print)
		cout<<cur_max<<'/'<<cnum<<endl;
	return cur_word;
}

int solver(){
	init();
	cout<<"Start"<<endl;
	int j=0;
	size_t cnum;
	word* clist;
#ifdef _WORDS_H
	size_t widx_init = cache[LEN];
#else
	size_t widx_init = find_word_multi(wordlist, cnum_all); //find_word(wordlist, cnum_all);
#endif
	size_t widx;
	const word* w;
	while(true){
		cnum = cnum_all;
		clist = wordlist;
		while(true){
			if(clist == wordlist){
				// widx = widx_init;
				w = wordlist + widx_init;
			}else{
				widx = find_word(clist, cnum);
				w = (widx < cnum_all)?clist+widx:wordlist+widx-cnum_all;
			}
			cout<<*w<<endl<<"Color:";
			colors c = 0;
			for(unsigned i=0; i<LEN; ++i){
				cin>>j;
				if(j == -1) break;
				c += cbit[i] * j;
			}
			if(j == -1) break;
			cnum = compress(*w, c, clist, cnum);
			if(cnum == 1){
				cout<<"Found: "<<cwlist[0]<<endl;
				cin>>j;
				break;
			}
			clist = cwlist;
		}
		//system("cls");
	}
	//end();
	//return 0;
}

int test_all(){
	init();
	cout<<"Start"<<endl;
	unsigned round=0, max_round = 0;
	unsigned long long tot_round = 0;
	size_t cnum;
	word* clist;
	ofstream out("result.txt");
	const word* w;
#ifdef _WORDS_H
	size_t widx_init = cache[LEN];
#else
	size_t widx_init = find_word_multi(wordlist, cnum_all, false);//find_word(wordlist, cnum_all, false);
#endif
	size_t widx;
	for(size_t j=0;j<cnum_all;++j){
		cnum = cnum_all;
		clist = wordlist;
		round = 0;
		while(true){
			++round;
			if(round == 1){
				// widx = widx_init;
				w = wordlist + widx_init;
			}else{
				widx = find_word(clist, cnum, false);
				w = (widx < cnum_all)?clist+widx:wordlist+widx-cnum_all;
			}
			//cout<<*w<<' '<<wordlist[j]<<endl;
			if(memcmp(w->alph, wordlist[j].alph, LEN) == 0){
				break;
			}
			colors c = get_color(*w, wordlist[j]);
			//cout<<"Color:"<<(int)c<<endl;
			cnum = compress(*w, c, clist, cnum);
			clist = cwlist;
		}
		tot_round += round;
		if(round > max_round) max_round = round;
		if(j % 100 == 99) cout<<j+1<<endl;
		out<<wordlist[j]<<'\t'<<round<<endl;
	}
	cout<<"average "<<tot_round * 1.0 /cnum_all<<" max "<<max_round<<endl;
	end();
	return 0;
}

int main(){
	//hardMode = true;
	//return test_all();
	return solver();
}
