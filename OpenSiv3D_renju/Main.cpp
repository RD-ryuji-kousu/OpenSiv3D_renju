 // Siv3D v0.6.14
#include "stdafx.h"
/*
* wall 壁 y, xが0未満や15以上になったとき返す用
* space 盤面の空白
* black　盤面の黒石
* white 盤面の白石
*/
enum class SquareState :unsigned char {
	wall, space, black, white
};

//盤上の線の定数
constexpr double LINE_STEP = 38.0;
constexpr double BORDER_SPACE = 35.0;
//相手の色を返す
inline SquareState next_color(SquareState c)
{
	return (c == SquareState::black) ? SquareState::white : SquareState::black;
}
//スタート画面
class Start {
public:
	void View() {
		font(first).draw(Arg::center(400, 200));
		font(second).draw(Arg::center(400, 400));
	}
private:
	const Font font{ 60 };
	String first = U"Choose first move?\n Press 'F'";
	String second = U"Choose second move?\n Press 'S'";
};

//ゲームオーバー画面
class Over {
public:
	/*
	@param[in] text1	勝利条件に則したテキストを受け取る
	*/
	void View(const String& text1) {
		title(text1).draw(Arg::center(400, 100), Palette::Red);
		font(restart).draw(Arg::center(400, 400));
		font(quit).draw(Arg::center(400, 500));
	}
private:
	const Font title{ 100 };
	const Font font{ 60 };
	String restart = U"Restart Press'R'";
	String quit = U"Quit Press 'Q'";
};
/*
ボードの描画
*/

class Board {
public:
	void Draw() {
		board.draw(Palette::Burlywood);
		for (double y = 0; y < 15; y++) {
			for (double x = 0; x < 15; x++) {
				Line{ board.x + x * LINE_STEP + BORDER_SPACE, board.y + y * LINE_STEP + BORDER_SPACE,
					board.rightX() - BORDER_SPACE,board.y + y * LINE_STEP + BORDER_SPACE}.draw(Palette::Black);
				Line{ board.x + x * LINE_STEP + BORDER_SPACE, board.y + y * LINE_STEP + BORDER_SPACE,
					board.x + x * LINE_STEP + BORDER_SPACE,
					board.bottomY()-BORDER_SPACE}.draw(Palette::Black);
			}
		}
	}
	/*
	* privateメンバーRectを返す
	*/
	const RectF& Get_board()const {
		return board;
	}
	RectF& Get_board() {
		return board;
	}
private:
	RectF board = { 100, 0, 600 };
};
/*盤上配列操作クラス
* 
* 
*/
class StonePos {
public:
	/*コンストラクタ
	*盤面管理配列boardをspaceで初期化する
	*/
	StonePos() {
		for (int y = 0; y < 15; y++) {
			for (int x = 0; x < 15; x++) {
				board[y][x] = SquareState::space;
			}
		}
	}
	/*
	* board の要素数オペレーター
	*/
	SquareState* operator[](int y) {
		return board[y];
	}
	const SquareState* operator[](int y)const {
		return board[y];
	}
	/*方向事の石が指定数あるかを判定する数
	* @param[in] c	石の色
	* @param[in] di		石の指定数
	* @param[in] (&score)[4]	石の数を格納した配列
	* @param[in] chktype		trueの時配列にdi以上の数が二つ以上あったときを判定する。falseのとき一以上を判定する。もしくは白の勝利判定か否か
	* @return bool falseのとき条件diを満たしていない。trueのとき条件diを満たしている
	*/
	bool chkcount(SquareState  c, int di, const int(&score)[4], bool chktype)const {
		int i;
		int count = 0;
		if (c == SquareState::black) {
			for (i = 0; i < 4; i++) {
				//連続した石の並びがdi以上の場合
				if (score[i] >= di) {
					//黒かつ連続した石の並びが5ピッタリの場合
					if (score[i] == di && c == SquareState::black && di == 5) {
						return true;
					}
					//黒かつ連続した石の並びが5を超える場合
					if (score[i] > 5 && c == SquareState::black && di > 5) {
						return true;
					}
					count++;
				}
				//連続した石の並びdi以上の物が二つ以上同時に出来た場合の判定
				if (count > 1 && chktype == true) {
					return true;
				}
				//連続した石の並びdi以上のものが一つでもあったときの判定
				if (chktype == false && count >= 1) {
					return true;
				}
			}

		}
		if (c == SquareState::white) {
			for (i = 0; i < 4; i++) {
				//連続した石の並びが5以上の場合
				if (score[i] >= di && di == 5 && chktype == true) {
					return true;
				}
				//連続した石の並びがdi以上の場合
				if (chktype == false && score[i] >= di) {
					return true;
				}
			}
		}
		return false;
	}
	/*縦横斜めの同色の石を数える関数
	* @param[in]	px	中心となる座標の横軸インデックスx
	* @param[in]	py	中心となる座標の縦軸インデックスy
	* @param[in]	dx	pxに対する検索範囲
	* @param[in]	dy	pyに対する検索範囲
	* @param[in]	c	検索対象の石の色
	* @return	上記情報によって見つかった整数を返す
	*/
	int IsSetDir(int px, int py, int dx, int dy, SquareState c) const {
		int count = 0;
		for (int y = py + dy, x = px + dx; y >= 0 && y < 15 && x >= 0 && x < 15; y += dy, x += dx) {
			if (board[y][x] != c) {
				return count;
			}
			count++;
		}
		return count;
	}
	/*
	* @param[in]	px	中心となる座標の横軸インデックスx
	* @param[in]	py	中心となる座標の縦軸インデックスy
	* @param[in]	c	使用される石の色
	* @param[out]	count	縦横斜めの連続した石の数を格納する
	* @return bool	石が置けるかどうか(true : false)
	*/
	bool IsSet(int px, int py, SquareState c, int(&count)[4])const {
		if (board[py][px] != SquareState::space) {//指定座標が空白でない場合
			for (int i = 0; i < 4; i++)//countの初期化
			{
				count[i] = 0;
			}
			return false;
		}
		int i = 0;
		int count_d[8];
		for (int dy = -1; dy <= 1; dy++) {
			for (int dx = -1; dx <= 1; dx++) {
				if (dx == 0 && dy == 0)continue;
				count_d[i] = IsSetDir(px, py, dx, dy, c);//8方向で石の数を取得
				i++;
			}
		}
		/*(y, x)
		* 0 (-1, -1) 1 (-1, 0) 2 (-1, 1)
		* 3 (0, -1)			   4(0, 1)
		* 5 (1, -1), 6 (1, 0) 7 (1, 1)
		*/
		//4方向化,自身の場所は情報がないため、+1
		count[0] = count_d[0] + count_d[7] + 1;	//(-1, -1), (1, 1)	//左上右下
		count[1] = count_d[1] + count_d[6] + 1;		//上下
		count[2] = count_d[2] + count_d[5] + 1;		//右上左下
		count[3] = count_d[3] + count_d[4] + 1;		//左右
		return true;
	}
	/*
	* @param[in]	px	 座標のインデックス横軸x
	* @param[in]	py	 座標のインデックス縦軸y
	* @param[in]	c	 適用する石の色
	* @param[out]	count	縦横斜めの連続した石の数を格納する
	* @return bool 石が置けた場合true,それ以外はfalse
	*/
	bool Set(int px, int py, SquareState c, int(&count)[4]){
		if (IsSet(px, py, c, count) == true) {
			board[py][px] = c;
			return true;
		}
		return false;
	}
	/*@brief 勝利判定
	* @param[in]	c	適用する石の色
	* @param[in] count	適用する連続した石の並び
	* @return bool 条件を満たしたとき true,満たしていない時falseを返す
	*/
	bool IssueWin(SquareState c, const int(&count)[4])const {
		if (c == SquareState::black) {
			if (chkcount(c, 5, count, true) == true) {
				return true;
			}
		}
		if (c == SquareState::white) {
			if (chkcount(c, 5, count, true)==true)return true;
		}
		return false;
	}
	/*@brief 敗北判定
	* @param[in]	c	適用する石の色
	* @param[in]	count	適用する石の並び
	* @return bool 条件をみたした時 true, 満たしていない時falseを返す
	*/
	bool IssueLose(SquareState c, const int (&count)[4])const {
		if (chkcount(c, 3, count, true) == true) {
			return true;
		}
		if (chkcount(c, 4, count, true) == true) {
			return true;
		}
		return false;
	}
	/*@brief 隣接、もしくは連続した石の隣接している物が空白、多色の石、壁かを調べる
	* @param[in]	count	適用する石の並び
	* @param[in]	px		中心となる座標のインデックスx
	* @param[in]	py		中心となる座標のインデックスy
	* @param[in]	di		調べる方向を決める基準di;
	* @return SquareState	隣接しているもののステータスを返す
	*/
	SquareState OtherChk(SquareState c, const int(&count)[4], int px, int py, int di)const {
		//与えられた座標下方向に壁もしくは多色の石があるかを検索
		if (di == 0) {
			for (int y = 0; y < count[1]; y++) {
				if (board[py + y + 1][px] != c && py + y + 1 < 15) {
					return board[py + y + 1][px];
				}
				if (py + y + 1 > 14) {
					return SquareState::wall;
				}
			}
		}
		//上方向に検索
		if (di == 1) {
			for (int y = 0; y < count[1]; y++) {
				if (board[py - (y + 1)][px] != c && py - (y + 1) >= 0) {
					return board[py - (y + 1)][px];
				}
				if (py - (y + 1) < 0) {
					return SquareState::wall;
				}
			}
		}
		//右方向に検索
		if (di == 2) {
			for (int x = 0; x < count[3]; x++) {
				if (board[py][px + (x + 1)] != c && px + x + 1 < 15) {
					return board[py][px + (x + 1)];
				}
				if (px + x + 1 > 14) {
					return SquareState::wall;
				}
			}
		}
		//左方向に検索
		if (di == 3) {
			for (int x = 0; x < count[3]; x++) {
				if (board[py][px - (x + 1)] != c && px - (x + 1) >= 0) {
					return board[py][px - (x + 1)];
				}
				if (px - (x + 1) < 0) {
					return SquareState::wall;
				}
			}
		}
		//左上方向に検索
		if (di == 4) {
			for (int x = 0, y = 0; (x < count[0] && y < count[0]); x++, y++) {
				if (board[py - (y + 1)][px - (x + 1)] != c && py - (y + 1) >= 0 && px - (x + 1) >= 0) {
					return board[py - (y + 1)][px - (x + 1)];
				}
				if (py - (y + 1) < 0 || px - (x + 1) < 0) {
					return SquareState::wall;
				}
			}
		}
		//右下方向に検索
		if (di == 5) {
			for (int x = 0, y = 0; (x < count[0] && y < count[0]); x++, y++) {
				if (board[py + (y + 1)][px + (x + 1)] != c && py + (y + 1) < 15 && px + x + 1 < 15) {
					return board[py + (y + 1)][px + (x + 1)];
				}
				if (py + (y + 1) > 14 || px + x + 1 > 14) {
					return SquareState::wall;
				}
			}
		}
		//右上方向に検索
		if (di == 6) {
			for (int x = 0, y = 0; (x < count[2] && y < count[2]); x++, y++) {
				if (board[py - (y + 1)][px + (x + 1)] != c && py - (y - 1) >= 0 && px + x + 1 < 15) {
					return board[py - (y + 1)][px + (x + 1)];
				}
				if (py - (y - 1) > 0 || px + x + 1 > 14) {
					return SquareState::wall;
				}
			}
		}
		//左下方向に検索
		if (di == 7) {
			for (int x = 0,  y = 0; (x < count[2] && y < count[2]); x++, y++) {
				if (board[py + (y + 1)][px - (x + 1)] != c && py + y + 1 < 15 || px - (x + 1) >= 0) {
					return board[py + (y + 1)][px - (x + 1)];
				}
				if (py + y + 1 > 14 || px - (x + 1) < 0) {
					return SquareState::wall;
				}
			}
		}
		return c;
	}
	/*
	* @brief	ボードを全て空白で埋める
	*/
	void reset() {
		for (int y = 0; y < 15; y++) {
			for (int x = 0; x < 15; x++) {
				board[y][x] = SquareState::space;
			}
		}
	}
protected:
	SquareState board[15][15];
};
class StoneMng {
public:

	/* @brief	配置された石の描画
	* @param[in]	bd	ボード情報
	*/
	void Draw(Board& bd) {
		for (int y = 0; y < 15; y++) {
			for (int x = 0; x < 15; x++) {
				SquareState f = board[y][x];
				if (f != SquareState::space) {
					Circle{ Vec2(bd.Get_board().x + BORDER_SPACE + (x * LINE_STEP)  ,
						bd.Get_board().y + BORDER_SPACE + y * LINE_STEP ),15 }.
						draw((f == SquareState::black) ? Palette::Black : Palette::White);
				}
			}
		}
	}
	bool IssueWin(SquareState c, int(&count)[4])const {
		return board.IssueWin(c, count);
	}
	bool IssueLose(SquareState c, int(&count)[4])const {
		return board.IssueLose(c, count);
	}
	/* @brief	マウスの左クリックされた地点を元に石を置く関数
	* @param[in]	bd	ボードの位置情報
	* @param[in]	pos	マウスの位置情報
	* @param[in]	c	石の色
	* @param[in]	count	連続した石の並び
	* @return 石が置けた場合true,そうでない場合falseを返す
	*/
	bool Set(Board& bd, const Point& pos, SquareState c, int(&count)[4]) {
		int x, y;
		if (pos.y < 30 || pos.y > 570 || pos.x < 140 || pos.x > 670) {
			return false;
		}
		x = static_cast<int>((pos.x - bd.Get_board().x - BORDER_SPACE + (LINE_STEP / 2)) / LINE_STEP);
		y = static_cast<int>((pos.y - bd.Get_board().y - BORDER_SPACE + (LINE_STEP / 2)) / LINE_STEP);
		if (x >= 0 && x < 15 && y >= 0 && y < 15) {
			if (board.Set(x, y, c, count) == true)return true;
		}
		return false;
	}
	/*
	* @param[in] x	点滅させる石のX座標
	* @param[in] y	点滅させる石のY座標
	* @param[in] c	点滅させる石の色
	* @param[in] desk	盤面の情報
	*/
	void SetandDraw(int x, int y, SquareState c, Board& desk) {
		board[y][x] = c;
		desk.Draw();
		Draw(desk);
		System::Sleep(0.5s);
		System::Update();
		ColorF color, icolor = {Palette::Lightpink};
		(c == SquareState::black) ? color =  { Palette::Black } : color = Palette::White;
		for (int i = 0; i < 4; i++) {
			desk.Draw();
			Draw(desk);
			Circle{ Vec2(desk.Get_board().x + BORDER_SPACE + (x * LINE_STEP)  ,
					desk.Get_board().y + BORDER_SPACE + y * LINE_STEP),15 }.
				draw((i % 2 == 0) ? color : icolor);
			System::Update();
			System::Sleep(0.5s);

		}
	}

	bool Compute(SquareState c, int turn, int(&count)[4], Board& desk);

	/*
	* 配列ボードを全てspaceで埋める
	*/
	void reset() {
		board.reset();
	}

protected:
	StonePos board;
};

//! CPUの思考クラス
class StoneComp :public StonePos {
	std::vector<StoneComp> next; // 次の 1手を打った盤面
	char px, py; // 本盤面 boardで打った位置(0~14, 0~14)
	SquareState put_c; // 本盤面 boardで打った色(black, white)
	int rate;		//打つ手の評価値
	static SquareState cpu_c;      // コンピュータ側の石の色
	static int Level0;
	static const int rate_numw[5];	//自ターンの壁なしレート// 自分の石の並び(1～5)に対する評価値（インデックスは並び-1）。※並びの片側が止められていない場合の評価値
	static const int rate_wnumw[5];	//自ターンの壁ありレート//自分の石の並び(1～5)に対する評価値（インデックスは並び-1）。※並びの片側が止められている場合の評価値
	static const int rate_numb[5];	//相手ターンの壁なしレート//相手の石の並び(1～5)に対する評価値（インデックスは並び-1）。※並びの片側が止められていない場合の評価値
	const static int rate_wnumb[5];	//相手ターンの壁ありレート//相手の石の並び(1～5)に対する評価値（インデックスは並び-1）。※並びの片側が止められている場合の評価値
public:
	/*コンストラクタ
	* param[in]	org	ボード配置の情報
	*/
	StoneComp(const StonePos& org) :px(0), py(0),
		put_c(SquareState::space), rate(0) {
		for (int y = 0; y < 15; y++) {
			for (int x = 0; x < 15; x++) {
				board[y][x] = org[y][x];
			}
		}
	}
	/*コンストラクタ
	* @param[in]	src	思考中ボード配置の情報
	*/
	StoneComp(const StoneComp& src) :next(), px(static_cast<char>(src.px)), py(static_cast<char>(src.py)), put_c(src.put_c),
		 rate(src.rate) {
		for (int y = 0; y < 15; y++) {
			for (int x = 0; x < 15; x++) {
				board[y][x] = src[y][x];
			}
		}
	}
	/*コンストラクタ
	* @param[in]	src 思考中ボードの位置情報
	* @param[in]	x	配置するX座標
	* @param[in]	y	配置するY座標
	* @param[in]	c	配置する石
	*/
	StoneComp(const StoneComp& src, int x, int y, SquareState c) :next(), px(static_cast<char>(x))
		, py(static_cast<char>(y)), put_c(c),
		rate(src.rate) {
		for (int j = 0; j < 15; j++) {
			for (int i = 0; i < 15; i++) {
				board[j][i] = src[j][i];
			}
		}
		int ratew[4] = { 0 }, rateb[4] = { 0 };
		if (c == SquareState::white) {
			if (IsSet(x, y, c, ratew) == true) {
				Set(x, y, c, ratew);
			}
		}
		if (c == SquareState::black) {
			if (IsSet(x, y, c, rateb) == true) {
				Set(x, y, c, rateb);
			}
		}
		
	}
	//レートの大小比較//クラスの大小比較は評価値の大小比較で行なう
	bool operator < (const StoneComp& rhs)const {
		return rate < rhs.rate;
	}
	/*レートの取得に置ける盤面情報の取得
	* @param[in] c	適用する石の色
	* @param[in] tx 調査する石のX座標
	* @param[in] ty 調査する石のY座標
	* @param[in] di 調査する石の並びの数
	* @param[in] dj 調査する石の方向
	* @param[in] ccount 調査する石の連続した並び
	* @return 0両端が空白, 1片方が空白, 2該当なし
	*/
	int get_rate(SquareState c, int tx, int ty, int di, int dj, const int (&ccount)[4]) {
		SquareState d = next_color(c);
		//指定された数di以上の石の並びがある場合
		if (chkcount(c, di, ccount, false) == true) {
			//両端が空白の場合
			if (OtherChk(c, ccount,tx, ty, dj) == SquareState::space
				&& OtherChk(c, ccount, tx, ty, dj + 1) == SquareState::space) {
					return 0;
			}
			//端の片方が多色の石や壁で埋まっているか
			if (OtherChk(c, ccount, tx, ty, dj) == SquareState::space
				&& OtherChk(c, ccount, tx, ty, dj + 1) == d)return 1;
			if (OtherChk(c, ccount, tx, ty, dj) == d
				&& OtherChk(c, ccount, tx, ty, dj + 1) == SquareState::space) return 1;
			if (OtherChk(c, ccount, tx, ty, dj) == SquareState::space
				&& OtherChk(c, ccount, tx, ty, dj + 1) == SquareState::wall)return 1;
			if (OtherChk(c, ccount, tx, ty, dj) == SquareState::wall
			&& OtherChk(c, ccount, tx, ty, dj + 1) == SquareState::space)return 1;
		}
		
		return 2;
	}

	/*!両方の石を置いたときの情報を取得
	* @param[in] x	調査する石のX座標
	* @param[in] y	調査する石のY座標
	* @param[in] c	調査する石の色
	* @param[in] level	何手先をみるか
	*/
	int IsSetC(int x, int y, SquareState c, int level) {
		int countc[4] = { 0 }, countd[4] = { 0 };//! 自分の石を配置した場合の縦横斜めの自石の連続数、相手の石を配置した場合の縦横斜めの相手石の連続数
		int ratec = 0, rated = 0;//! 自分の石を配置した場合の評価値、相手の石を配置した場合の評価値
		SquareState d = next_color(c);
		if (IsSet(x, y, c, countc) == true && IsSet(x, y, d, countd)==true) {
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 8; j += 2) {
					//自分の手の両端が空白かチェックまた数によってレートを加算
					if (get_rate(c, x, y, i + 1, j, countc) == 0) {
						if (c == SquareState::black && chkcount(c, i, countc, true) == true && i >= 3) {
							//色が黒かつ敗北条件を見たいている場合減算
							ratec +=  -rate_numw[i - (level/2)];
						}
						else ratec +=  rate_numw[i - (level / 2)];
					}
					//片方が埋まっている場合
					if (get_rate(c, x, y, i + 1, j, countc) == 1) {
						if (c == SquareState::black && chkcount(c, i, countc, true) == true && i >= 3) {
							ratec +=  -rate_wnumw[i - (level / 2)]; 
						}
						else ratec += rate_wnumw[i - (level / 2)]; 
					}
					//相手の色の両端空白チェック
					if(get_rate(d, x, y, i + 1, j, countd) == 0) {
						if (d == SquareState::black && chkcount(d, i, countd, true) == true && i >= 3) {
							rated +=  -rate_numb[i - (level / 2)];
						}
						else rated +=  rate_numb[i - (level / 2)];
					}
					//相手の色の片方が埋まっている場合
					if (get_rate(d, x, y, i + 1, j, countd) == 1) {
						if (d == SquareState::black && chkcount(d, i, countd, true) == true && i >= 3) {
							rated +=  -rate_wnumb[i - (level / 2)];
						}
						else rated += rate_wnumb[i - (level / 2)];
					}
				}
			}
		}
		return std::max(ratec, rated);	// 自分の石と相手の石で評価値の高い方を返す

	}

	/*
	* param[in]		x	調査する石のX座標
	* param[in]		y	調査する石のY座標
	* param[in]		c	調査する石の色
	* param[in]		level	先の手を見る上での掘り下げ階層
	*/
	void addnext(int x, int y, SquareState c, int level) {
		int rate_t = 0;

		next.push_back(StoneComp(*this, x, y, c));
		rate_t = IsSetC(x, y, c, level);
		next.back().rate = rate_t;
		//Print << next.back().rate;
	}

	int compute_sub(int level, SquareState c, int turn);
	void compute(int level, SquareState c, int& x, int& y, int turn);
};

SquareState StoneComp::cpu_c = SquareState::black;
int StoneComp::Level0 = 0;
const int StoneComp::rate_numw[5] = { 1,  3, 14, 214, 214 };
const int StoneComp::rate_wnumw[5] = { -1, 0, 2, 54, 214 };
const int StoneComp::rate_numb[5] = { 1,  2, 13, 1060, 4232 };
const int StoneComp::rate_wnumb[5] = { -2, -1,1,13,215 };


/* 
* @param[in] level	何手先をみるかでの掘り下げ階層
* @param[in] c		調査する石の色
* @param[in] turn	現在のターン数
* @return 最大のレートを返す
*/
int StoneComp::compute_sub(int level, SquareState c, int turn) {
	SquareState d = next_color(c);
	int ccount[4] = { 0 };
	if (turn != 0) {
		//全ての座標を精査
		for (int y = 0; y < 15; y++) {
			for (int x = 0; x < 15; x++) {
				//ボードの空白を見つける
				if (board[y][x] == SquareState::space) {
					//空白から八方向検索
					for (int i = -1; i <= 1; i++) {
						for (int j = -1; j <= 1; j++){
							if (i == 0 && j == 0)continue;
							if (x + j < 15 && x + j >= 0 && y + i < 15 && y + i >= 0) {
								//いずれかの場所に石があった場合レートを計算
								if (board[y + i][x + j] == c || board[y + i][x + j] == d) {
									if (IsSet(x, y, c, ccount) == true) addnext(x, y, c, level);
									if (level < Level0) {
										next.back().rate = next.back().compute_sub(level + 1, next_color(c), turn + 1);
									}
									//二重ループを抜ける
									goto bp1;
									
								}
							}
						}
					}
				bp1:;
					


				}
			}
		}
	}
	else {//cpuが最初のターンの場合
		if (IsSet(7, 7, c, ccount) == true)addnext(7, 7, c, level);
		if (level < Level0) {
			next.back().rate = next.back().compute_sub(level + 1, next_color(c), turn + 1);
		}
	}
	return std::max_element(next.begin(), next.end())->rate;
}

/*
* @param[in] level	何手先をみるか
* @param[in] c		自身の石の色
* @param[out] x		cpuが置く場所のX座標
* @param[out] y		cpuが置く場所のY座標
* @param[in] turn	現在のターン数
*/
void StoneComp::compute(int level, SquareState c, int& x, int& y, int turn) {
	Level0 = level;
	if(turn == 0)
	cpu_c = c;
	compute_sub(0, c, turn);
	auto it = std::max_element(next.begin(), next.end());
	x = it->px;
	y = it->py;

	//Rate_Print();
}

/*
* @param[in] c		自身の石の色
* @param[in] turn	現在のターン数
* @param[out] count	連続した石の並び
* @param[out] desk
* @return 石が置けた場合true,石が置けなかった場合false
*/
bool StoneMng::Compute(SquareState c, int turn, int(&count)[4], Board& desk) {
	StoneComp cpu(board);
	int x, y;
	cpu.compute(0, c, x, y, turn);
	if (x >= 0 && x < 15 && y >= 0 && y < 15) {
		if (board.IsSet(x, y, c, count) == true) {			
			SetandDraw(x, y, c, desk);
			return true;
		}
	}
	return false;
}

void Main()
{
	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{ 0.0, 0.0, 0.0 });
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);
	Board board;
	StoneMng stone;
	Start gamebegin;
	Over gameover;
	SquareState c = SquareState::space, cpuc = SquareState::space;
	int turn = 0, plside = 0;
	int count[4] = {0};
	String Game_state = U"Start";
	Stopwatch stpwtch{ StartImmediately::No };
	Font side{ 20 };
	String text1;

	while (System::Update())
	{
		//ゲーム開始処理
		if (Game_state == U"Start") {
			gamebegin.View();
			//Fで人間が先手
			if (KeyF.pressed()) {
				plside = 0;
				cpuc = SquareState::white;
				Game_state = U"game";
			}
			//Sで人間が後手
			else if (KeyS.pressed()) {
				plside = 1;
				cpuc = SquareState::black;
				Game_state = U"game";
			}
		}
		//ゲームプレイ中処理
		else if (Game_state == U"game") {
			c = (turn % 2 == 0) ? SquareState::black : SquareState::white;
			
			if (plside == turn % 2) {
				side(U"Your turn").draw(0, 570);
				if (MouseL.down()) {
					if (stone.Set(board, Cursor::Pos(), c, count) == true) {
						turn++;
						side(U"CPU turn").draw(0, 570);
					}
				}
			}
			else {
				System::Sleep(2s);
				if (stone.Compute(cpuc, turn, count, board) == true) {
					
					//System::Sleep(1s);
					turn++;
				}
			}
			if (stone.IssueWin(c, count) == true) {
				if (c == SquareState::black) {
					text1 = U"Black Win";
				}
				else {
					text1 = U"White Win";
				}
				Game_state = U"over";
			}
			if (stone.IssueLose(c, count) == true) {
				text1 = U"White Win";
				Game_state = U"White Win";
			}
			board.Draw();
			stone.Draw(board);
		}
		//ゲームオーバー処理
		else {
			gameover.View(text1);
			//Rでリスタート
			if (KeyR.pressed()) {
				stone.reset();
				turn = 0;
				for (int i = 0; i < 4; i++)count[i] = 0;
				Game_state = U"Start";
			}
			//Qで終了
			if (KeyQ.pressed()) {
				System::Exit();
			}

		}
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
