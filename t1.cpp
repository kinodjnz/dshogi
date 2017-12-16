#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>
#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

enum class animal_t {
	L, P, K, Z, N, E
};

typedef long long ll;
using std::printf;

bool skip_table[54] = {
	true,  true,  false,
	       true,  false,
	true,  true,  false,
	false, false, false,

	false, false, false,
	true,  true,  false,
	       true,  false,
	true,  true,  false,

	false, false, false,
	false, false, false,
	true,  true,  false,
	       true,  false,

	true,  true,
	true,
	true,  true,
	false, false,

	false, false,
	true,  true,
	true,
	true,  true,

	false, false,
	false, false,
	true,  true,
	true,
};

int table_map[54];
char *win_counts; // 11878227 * 28

int get_win_count(int id) {
	int x = id/11878227;
	int j = table_map[x];
	if (j == -1)
		return 1;
	return win_counts[j*11878227 + id%11878227];
}

class game {
public:
	//typedef long long case_t;
	typedef int case_t;
private:
	struct cell_t {
		cell_t() : animal(animal_t::E), dir(false) {}
		cell_t(animal_t a_animal, bool a_dir)
			: animal(a_animal), dir(a_dir)
			{}

		animal_t animal;
		bool dir;
	};
	struct state_t {
		int type;
		bool dir;
		int x;
		int y;
		bool is_niwatori;

		state_t() {}
	private:
		state_t(int a_type, bool a_dir, int a_x, int a_y, bool a_is_niwatori)
			: type(a_type), dir(a_dir), x(a_x), y(a_y),
			  is_niwatori(a_is_niwatori)
			{}

	public:
		/*
		static state_t new_hand(bool dir) {
			return state_t(1, dir, 0, 0, false);
		}
		*/
		static void new_hand(state_t *s, bool dir) {
			s->type = 1;
			s->dir = dir;
			//s->x = 0;
			//s->y = 0;
			s->is_niwatori = false;
		}
		/*
		static state_t new_on_board(bool dir, int x,  int y,
									bool is_niwatori = false) {
			return state_t(0, dir, x, y, is_niwatori);
		}
		*/
		static void new_on_board(state_t *s, bool dir, int x,  int y,
								 bool is_niwatori = false) {
			s->type = 0;
			s->dir = dir;
			s->x = x;
			s->y = y;
			s->is_niwatori = is_niwatori;
		}
	};
	typedef std::array<int, 8> ind_t;
	//typedef std::map<animal_t, std::vector<state_t> > hash_t;
	//typedef std::array<std::vector<state_t>, 4> hash_t;
	class hash_t {
	public:
		typedef std::array<state_t, 2> value_type;
		typedef std::array<value_type, 4>::iterator iterator;
		typedef std::array<value_type, 4>::const_iterator const_iterator;
	private:
		std::array<value_type, 4> v_;
	public:
		value_type &operator[](animal_t ani) {
			return v_[static_cast<int>(ani)];
		}
		const value_type &operator[](animal_t ani) const {
			return v_[static_cast<int>(ani)];
		}
		iterator begin() { return v_.begin(); }
		iterator end() { return v_.end(); }
		const_iterator begin() const { return v_.begin(); }
		const_iterator end() const { return v_.end(); }
	};
	class hands_t {
		std::array<animal_t, 8> v_;
		size_t n_;
	public:
		hands_t() : n_(0) {}
		animal_t operator[](size_t i) const {
			return v_[i];
		}
		animal_t &operator[](size_t i) {
			return v_[i];
		}
		void push_back(animal_t c) {
			v_[n_++] = c;
		}
		size_t size() const {
			return n_;
		}
		void erase(size_t i) {
			std::copy(&v_[i+1], &v_[n_], &v_[i]);
			n_--;
		}
		void insert(size_t i, animal_t c) {
			n_++;
			std::copy_backward(&v_[i], &v_[n_], &v_[n_+1]);
			v_[i] = c;
		}
		void pop_back() {
			n_--;
		}
		void clear() {
			n_ = 0;
		}
		const animal_t *begin() const {
			return &v_[0];
		}
		const animal_t *end() const {
			return &v_[n_];
		}
		animal_t *begin() {
			return &v_[0];
		}
		animal_t *end() {
			return &v_[n_];
		}
	};

	cell_t board_[12];
	hands_t hands_[2];

	case_t count_case(int m, int /*b*/) {
		//if (b == 1)
		return (ll)2*m*m + 2*m + 3;
		//return 2*m*(m-1)*f(m-2, b-1) + 4*m*f(m-1, b-1) + 3*f(m, b-1);
	}
	case_t encode_lion(const state_t &lion1, const state_t &lion2, ind_t &ind) {
		case_t r;
		ind[0] = 3*lion1.y + lion1.x;
		ind[1] = 3*lion2.y + lion2.x;
		if (lion1.x == 0) {
			if (ind[0] < ind[1])
				ind[1]--;
			r = (lion1.y-1)*11 + ind[1];
		} else {
			int ind1 = 2*lion1.y + lion1.x;
			int ind2 = 2*lion2.y + lion2.x;
			if (ind1 < ind2)
				ind2--;
			r = 33 + (lion1.y-1)*7 + ind2;
		}
		return r;
	}
	bool update_ind(const state_t &ani1, const state_t &ani2,
					ind_t &ind, int j) {
		bool swapped = false;
		int ind1 = 3*ani1.y + ani1.x;
		int ind2 = 3*ani2.y + ani2.x;
		if (ind1 > ind2) {
			swapped = true;
			std::swap(ind1, ind2);
		}
		for (int k = 0; k < j; k++) {
			if (ind[k] <= ind1) ind1--;
			if (ind[k] <= ind2) ind2--;
		}
		ind2--;
		ind[j] = ind1;
		ind[j+1] = ind2;
		return swapped;
	}
	case_t encode_hands(const state_t &ani1, const state_t &ani2) {
		case_t x;
		if (ani1.dir && ani2.dir)
			x = 2;
		else if (!ani1.dir && !ani2.dir)
			x = 0;
		else
			x = 1;
		return x;
	}
	case_t encode_zo(const hash_t &h, ind_t &ind, int m) {
		const state_t *zo1 = &h[animal_t::Z][0];
		const state_t *zo2 = &h[animal_t::Z][1];
		case_t r;
		int t = zo1->type + zo2->type;
		if (t == 0) {
			if (update_ind(*zo1, *zo2, ind, 6))
				std::swap(zo1, zo2);
			r = zo1->dir+zo2->dir*2;
			r = r + 4*((m*(m-1)/2-(m-1-ind[6])*(m-ind[6])/2) + (ind[7]-ind[6]));
		} else if (t == 1) {
			if (zo1->type == 1)
				std::swap(zo1, zo2);
			ind[6] = 3*zo1->y + zo1->x;
			ind[7] = 9999;
			for (int k = 0; k < 6; k++) {
				if (ind[k] <= ind[6]) ind[6]--;
			}
			r = zo1->dir;
			r = r + 2*(ind[6] + m*zo2->dir);
			r = 2*m*(m-1) + r;
		} else {
			case_t x = encode_hands(*zo1, *zo2);
			r = 2*m*(m-1) + 4*m + x;
		}
		return r;
	}
	case_t encode_kirin(const hash_t &h, ind_t &ind, int m) {
		const state_t *kirin1 = &h[animal_t::K][0];
		const state_t *kirin2 = &h[animal_t::K][1];
		case_t r;
		int t = kirin1->type + kirin2->type;
		if (t == 0) {
			if (update_ind(*kirin1, *kirin2, ind, 4))
				std::swap(kirin1, kirin2);
			r = kirin1->dir+kirin2->dir*2;
			r = r + 4*((m*(m-1)/2-(m-1-ind[4])*(m-ind[4])/2) + (ind[5]-ind[4]));
			r = r + 2*m*(m-1)*encode_zo(h, ind, m-2);
		} else if (t == 1) {
			if (kirin1->type == 1)
				std::swap(kirin1, kirin2);
			ind[4] = 3*kirin1->y + kirin1->x;
			ind[5] = 9999;
			for (int k = 0; k < 4; k++)
				if (ind[k] <= ind[4]) ind[4]--;
			r = kirin1->dir;
			r = r + 2*(ind[4] + m*kirin2->dir);
			r = 2*m*(m-1)*count_case(m-2, 1) + r;
			r += 4*m*encode_zo(h, ind, m-1);
		} else {
			case_t x = encode_hands(*kirin1, *kirin2);
			r = 2*m*(m-1)*count_case(m-2, 1) + 4*m*count_case(m-1, 1) + x;
			r += 3*encode_zo(h, ind, m);
		}
		return r;
	}
	case_t encode_piyo(const hash_t &h, ind_t &ind) {
		const state_t *piyo1 = &h[animal_t::P][0];
		const state_t *piyo2 = &h[animal_t::P][1];
		case_t r;
		int t = piyo1->type + piyo2->type;
		if (t == 0) {
			if (update_ind(*piyo1, *piyo2, ind, 2))
				std::swap(piyo1, piyo2);
			r = piyo1->dir + piyo1->is_niwatori*2 +
				piyo2->dir*4 + piyo2->is_niwatori*8;
			r = r + 16*((45-(9-ind[2])*(10-ind[2])/2) + (ind[3]-ind[2]));
			r = r + 16*45*encode_kirin(h, ind, 8);
		} else if (t == 1) {
			if (piyo1->type == 1)
				std::swap(piyo1, piyo2);
			ind[2] = 3*piyo1->y + piyo1->x;
			ind[3] = 9999;
			if (ind[0] <= ind[2]) ind[2]--;
			if (ind[1] <= ind[2]) ind[2]--;
			r = piyo1->dir + piyo1->is_niwatori*2;
			r = r + 4*(ind[2] + 10*piyo2->dir);
			r = 13865 * (16*45) + r + 4*10*2*encode_kirin(h, ind, 9);
		} else {
			case_t x = encode_hands(*piyo1, *piyo2);
			r = 13865 * (16*45) + 22401 * (4*10*2) + x;
			r += 3*encode_kirin(h, ind, 10);
		}
		return r;
	}
public:
	std::pair<case_t, bool> board_id() {
		hash_t h;
		int hi[4] = {};
		for (int i = 0; i < 12; i++) {
			animal_t c = board_[i].animal;
			if (c == animal_t::E)
				continue;
			bool niwatori = false;
			if (c == animal_t::N) {
				c = animal_t::P;
				niwatori = true;
			}
			bool dir = board_[i].dir;
			//h[c].push_back(state_t::new_on_board(dir, i%3, i/3, niwatori));
			state_t::new_on_board(&h[c][hi[(int)c]++], dir, i%3, i/3, niwatori);
		}
		for (int i = 0; i < 2; i++) {
			for (auto it = hands_[i].begin(); it != hands_[i].end(); ++it)
				state_t::new_hand(&h[*it][hi[(int)*it]++], i);
		}
		if (h[animal_t::L][0].dir) {
			std::swap(h[animal_t::L][0], h[animal_t::L][1]);
		}
		bool flipx = false;
		auto &lion = h[animal_t::L];
		if (lion[0].x == 2 || (lion[0].x == 1 && lion[1].x == 2)) {
			// flip
			flipx = true;
			for (auto it = h.begin(); it != h.end(); ++it) {
				for (int j = 0; j < 2; j++) {
					if ((*it)[j].type == 0)
						(*it)[j].x = 2-(*it)[j].x;
				}
			}
		}

		ind_t ind;
		for (int i = 0; i < static_cast<int>(ind.size()); i++)
			ind[i] = 9999;
		case_t r_lion = encode_lion(h[animal_t::L][0], h[animal_t::L][1], ind);
		case_t r_piyo = encode_piyo(h, ind);
		case_t r = r_lion*11878227 + r_piyo/*54*/;
		return std::make_pair(r, flipx);
	}

private:
	void decode_lion(hash_t &h, case_t r, ind_t &ind) {
		state_t::new_on_board(&h[animal_t::L][0], false, 0, 0);
		state_t::new_on_board(&h[animal_t::L][1], true, 0, 0);
		state_t &a1 = h[animal_t::L][0];
		state_t &a2 = h[animal_t::L][1];
		if (r < 33) {
			a1.x = 0;
			a1.y = r/11+1;
			int ind1 = r%11;
			if (ind1 >= 3*a1.y+a1.x)
				ind1++;
			a2.x = ind1%3;
			a2.y = ind1/3;
		} else {
			r -= 33;
			a1.x = 1;
			a1.y = r/7+1;
			int ind1 = r%7;
			if (ind1 >= 2*a1.y+a1.x)
				ind1++;
			a2.x = ind1%2;
			a2.y = ind1/2;
		}
		ind[0] = 3*a1.y + a1.x;
		ind[1] = 3*a2.y + a2.x;
		if (ind[0] < ind[1])
			ind[1]--;
	};
	void decode_hands(hash_t &h, case_t r, animal_t ani) {
		if (r == 0) {
			state_t::new_hand(&h[ani][0], false);
			state_t::new_hand(&h[ani][1], false);
		} else if (r == 1) {
			state_t::new_hand(&h[ani][0], false);
			state_t::new_hand(&h[ani][1], true);
		} else if (r == 2) {
			state_t::new_hand(&h[ani][0], true);
			state_t::new_hand(&h[ani][1], true);
		}
	};
	std::pair<int, int> decode_ind(case_t r, ind_t &ind, int j, int m) {
		int ind1 = 0;
		int ind2 = 0;
		for (int s = m-1; s > 0; s--) {
			if (r < s) {
				ind2 = r + ind1;
				break;
			}
			ind1++;
			r -= s;
		}
		ind[j] = ind1;
		ind[j+1] = ind2;
		ind2++;
		for (int i = j-1; i >= 0; i--) {
			if (ind[i] <= ind1) ind1++;
			if (ind[i] <= ind2) ind2++;
		}
		return std::make_pair(ind1, ind2);
	};
	void decode_zo(hash_t &h, case_t r, ind_t &ind, int m) {
		if (r < 2*m*(m-1)) {
			state_t::new_on_board(&h[animal_t::Z][0], false, 0, 0);
			state_t::new_on_board(&h[animal_t::Z][1], false, 0, 0);
			state_t &a1 = h[animal_t::Z][0];
			state_t &a2 = h[animal_t::Z][1];
			a1.dir = (r&1);
			a2.dir = (r&2);
			r = r/4;
			std::pair<int, int> indr = decode_ind(r, ind, 6, m);
			a1.x = indr.first%3;
			a1.y = indr.first/3;
			a2.x = indr.second%3;
			a2.y = indr.second/3;
		} else if (r < 2*m*(m-1) + 4*m) {
			state_t::new_on_board(&h[animal_t::Z][0], false, 0, 0);
			state_t::new_hand(&h[animal_t::Z][1], false);
			state_t &a1 = h[animal_t::Z][0];
			state_t &a2 = h[animal_t::Z][1];
			r -= 2*m*(m-1);
			a1.dir = (r&1);
			r = r/2;
			int ind1 = r%m;
			ind[6] = ind1;
			for (int i = 5; i >= 0; i--)
				if (ind[i] <= ind1) ind1++;
			a1.x = ind1%3;
			a1.y = ind1/3;
			a2.dir = r/m;
		} else {
			r -= 2*m*(m-1) + 4*m;
			decode_hands(h, r, animal_t::Z);
		}
	}
	void decode_kirin(hash_t &h, case_t r, ind_t &ind, int m) {
		case_t offset = 2*m*(m-1)*count_case(m-2, 1);
		if (r < offset) {
			state_t::new_on_board(&h[animal_t::K][0], false, 0, 0);
			state_t::new_on_board(&h[animal_t::K][1], false, 0, 0);
			state_t &a1 = h[animal_t::K][0];
			state_t &a2 = h[animal_t::K][1];
			case_t r_zo = r/(2*m*(m-1));
			r = r % (2*m*(m-1));
			a1.dir = (r&1);
			a2.dir = (r&2);
			r = r/4;
			std::pair<int, int> indr = decode_ind(r, ind, 4, m);
			a1.x = indr.first%3;
			a1.y = indr.first/3;
			a2.x = indr.second%3;
			a2.y = indr.second/3;
			decode_zo(h, r_zo, ind, m-2);
		} else {
			case_t offset2 = offset + 4*m*count_case(m-1, 1);
			if (r < offset2) {
				state_t::new_on_board(&h[animal_t::K][0], false, 0, 0);
				state_t::new_hand(&h[animal_t::K][1], false);
				state_t &a1 = h[animal_t::K][0];
				state_t &a2 = h[animal_t::K][1];
				r -= offset;
				case_t r_zo = r/(4*m);
				r = r%(4*m);
				a1.dir = (r&1);
				r = r/2;
				int ind1 = r%m;
				ind[4] = ind1;
				for (int i = 3; i >= 0; i--)
					if (ind[i] <= ind1) ind1++;
				a1.x = ind1%3;
				a1.y = ind1/3;
				a2.dir = r/m;
				decode_zo(h, r_zo, ind, m-1);
			} else {
				r -= offset2;
				case_t r_zo = r/3;
				r = r%3;
				decode_hands(h, r, animal_t::K);
				decode_zo(h, r_zo, ind, m);
			}
		}
	}
	void decode_piyo(hash_t &h, case_t r, ind_t &ind) {
		if (r < 13865 * (16*45)) {
			state_t::new_on_board(&h[animal_t::P][0], false, 0, 0);
			state_t::new_on_board(&h[animal_t::P][1], false, 0, 0);
			state_t &a1 = h[animal_t::P][0];
			state_t &a2 = h[animal_t::P][1];
			case_t r_kirin = r/(16*45);
			r = r%(16*45);
			a1.dir = (r&1);
			a1.is_niwatori = (r&2);
			a2.dir = (r&4);
			a2.is_niwatori = (r&8);
			r = r/16;
			std::pair<int, int> indr = decode_ind(r, ind, 2, 10);
			a1.x = indr.first%3;
			a1.y = indr.first/3;
			a2.x = indr.second%3;
			a2.y = indr.second/3;
			decode_kirin(h, r_kirin, ind, 8);
		} else if (r < 13865 * (16*45) + 22401 * (4*10*2)) {
			state_t::new_on_board(&h[animal_t::P][0], false, 0, 0);
			state_t::new_hand(&h[animal_t::P][1], false);
			state_t &a1 = h[animal_t::P][0];
			state_t &a2 = h[animal_t::P][1];
			r -= 13865 * (16*45);
			case_t r_kirin = r/(4*10*2);
			r = r % (4*10*2);
			a1.dir = (r&1);
			a1.is_niwatori = (r&2);
			r = r/4;
			int ind1 = r%10;
			ind[2] = ind1;
			for (int i = 1; i >= 0; i--)
				if (ind[i] <= ind1) ind1++;
			a1.x = ind1%3;
			a1.y = ind1/3;
			a2.dir = r/10;
			decode_kirin(h, r_kirin, ind, 9);
		} else {
			r -= 13865 * (16*45) + 22401 * (4*10*2);
			case_t r_kirin = r/3;
			r = r%3;
			decode_hands(h, r, animal_t::P);
			decode_kirin(h, r_kirin, ind, 10);
		}
	}
public:
	void from_board_id(case_t id, bool flipx) {
		case_t r_lion = id / 11878227;//id % 54;
		case_t r_piyo = id % 11878227;//id / 54;
		hash_t h;
		ind_t ind;
		for (int i = 0; i < static_cast<int>(ind.size()); i++)
			ind[i] = 9999;
		decode_lion(h, r_lion, ind);
		decode_piyo(h, r_piyo, ind);
		std::fill(board_, board_+12, cell_t());
		hands_[0].clear();
		hands_[1].clear();
		auto update = [flipx, this](state_t &ani, animal_t ch) {
			int ind;
			if (flipx)
				ind = 3*ani.y + (2-ani.x);
			else
				ind = 3*ani.y + ani.x;
			if (ani.type == 0) {
				board_[ind].animal = ch;
				board_[ind].dir = ani.dir;
			} else if (ani.dir)
				hands_[1].push_back(ch);
			else
				hands_[0].push_back(ch);
		};
		update(h[animal_t::L][0], animal_t::L);
		update(h[animal_t::L][1], animal_t::L);
		update(h[animal_t::P][0],
			   (h[animal_t::P][0].is_niwatori?animal_t::N:animal_t::P));
		update(h[animal_t::P][1],
			   (h[animal_t::P][1].is_niwatori?animal_t::N:animal_t::P));
		update(h[animal_t::K][0], animal_t::K);
		update(h[animal_t::K][1], animal_t::K);
		update(h[animal_t::Z][0], animal_t::Z);
		update(h[animal_t::Z][1], animal_t::Z);
	}

	void print() {
		char chmap[] = "LPKZN ";
		for (auto it = hands_[1].begin(); it != hands_[1].end(); ++it)
			printf("%c", chmap[static_cast<int>(*it)]);
		printf("\n");
		for (int y = 0; y < 4; y++) {
			printf("-------\n");
			for (int x = 0; x < 3; x++) {
				cell_t &c = board_[3*y+x];
				if (c.dir || c.animal == animal_t::E)
					printf("|%c", chmap[static_cast<int>(c.animal)]);
				else
					printf("|^");
			}
			printf("|\n");
			for (int x = 0; x < 3; x++) {
				cell_t &c = board_[3*y+x];
				if (c.dir && c.animal != animal_t::E)
					printf("|v");
				else
					printf("|%c", chmap[static_cast<int>(c.animal)]);
			}
			printf("|\n");
		}
		printf("-------\n");
		for (auto it = hands_[0].begin(); it != hands_[0].end(); ++it)
			printf("%c", chmap[static_cast<int>(*it)]);
		printf("\n");
	}

	// returns 1 if 1st win
	//        -1 if 1st loose
	//         0 if otherwise.
	int win_or_loose_leaf() {
		int lion = 0;
		for (int i = 0; i < 12; i++) {
			if (board_[i].animal == animal_t::L && board_[i].dir) {
				lion = i;
				break;
			}
		}
		int x1 = lion%3;
		int y1 = lion/3;
		for (int i = 0; i < 12; i++) {
			if (i == lion)
				continue;
			if (!board_[i].dir) {
				int x2 = i%3;
				int y2 = i/3;
				if (board_[i].animal == animal_t::L) {
					if (abs(x1-x2) <= 1 && abs(y1-y2) <= 1)
						return 1;
				}
				if (board_[i].animal == animal_t::P) {
					if (x1==x2 && y1+1==y2)
						return 1;
				}
				if (board_[i].animal == animal_t::N) {
					if ((abs(x1-x2) <= 1 && (y2-y1 == 0 || y2-y1 == 1)) ||
						(x1==x2 && y2+1==y1))
						return 1;
				}
				if (board_[i].animal == animal_t::K) {
					if ((x1==x2 && abs(y1-y2)==1) ||
						(y1==y2 && abs(x1-x2)==1))
						return 1;
				}
				if (board_[i].animal == animal_t::Z) {
					if (abs(x1-x2)==1 && abs(y1-y2)==1)
						return 1;
				}
			}
		}
		if (y1 == 3)
			return -1;
		return 0;
	}

	void flipy() {
		for (int i = 0; i < 6; i++) {
			int x = i%3;
			int y = i/3;
			int j = 3*(3-y)+x;
			std::swap(board_[i], board_[j]);
			if (board_[i].animal != animal_t::E)
				board_[i].dir = !board_[i].dir;
			if (board_[j].animal != animal_t::E)
				board_[j].dir = !board_[j].dir;
		}
		std::swap(hands_[0], hands_[1]);
	}

	struct pos {
		pos() : x(15), y(15) {}
		pos(int ax, int ay) : x(ax), y(ay) {}
		int x;
		int y;
	};
	const pos *animal_moves(animal_t animal) {
		static const pos moves[][10] = {
			// L
			{
				pos(-1, -1), pos(0, -1), pos(1, -1),
				pos(-1, 0), pos(1, 0),
				pos(-1, 1), pos(0, 1), pos(1, 1)
			},
			// P
			{
				pos(0, -1)
			},
			// K
			{
				pos(0, -1),
				pos(-1, 0), pos(1, 0),
				pos(0, 1)
			},
			// Z
			{
				pos(-1, -1), pos(1, -1),
				pos(-1, 1), pos(1, 1)
			},
			// N
			{
				pos(-1, -1), pos(0, -1), pos(1, -1),
				pos(-1, 0), pos(1, 0),
				pos(0, 1)
			},
			// E
			{
			}
		};
		return moves[(int)animal];
	}

	int win_or_loose() {
		//std::pair<case_t, bool> id2 = board_id();
		flipy();
		//print();
		int res = -1;
		for (int i = 0; i < 12; i++) {
			if (board_[i].dir) {
				if (board_[i].animal == animal_t::E || !board_[i].dir)
					continue;
				int x = i%3;
				int y = i/3;
				animal_t animal = board_[i].animal;
				const pos *moves = animal_moves(animal);
				for (; moves->x <= 1; moves++) {
#define P() printf("i=%d m(%d,%d) c=%d\n", i, moves->x, moves->y, c)
					int y2 = y - moves->y;
					if (y2 < 0 || 4 <= y2)
						continue;
					int x2 = x + moves->x;
					if (x2 < 0 || 3 <= x2)
						continue;
					int k = y2*3+x2;
					cell_t orig = board_[k];
					if (orig.dir)
						continue;
					board_[k].animal = animal;
					board_[k].dir = true;
					if (y2 == 3 && animal == animal_t::P)
						board_[k].animal = animal_t::N;
					board_[i].animal = animal_t::E;
					board_[i].dir = false;
					if (orig.animal != animal_t::E) {
						animal_t ani = orig.animal;
						if (ani == animal_t::N)
							ani = animal_t::P;
						hands_[1].push_back(ani);
					}
					{
						//int c = -2;
						//P();
						//print();
						//getchar();
					}
					std::pair<case_t, bool> id = board_id();
					int c = get_win_count(id.first);
					if (c < 0) {
						//P();
						return 1;
					}
					if (c == 0) {
						//P();
						//return 0;
						res = 0;
					}
					board_[i].animal = animal;
					board_[i].dir = true;
					board_[k] = orig;
					if (orig.animal != animal_t::E)
						hands_[1].pop_back();
				}
			}
		}

		animal_t prev = animal_t::E;
		for (std::size_t i = 0; i < hands_[1].size(); i++) {
			if (prev == hands_[1][i])
				continue;
			prev = hands_[1][i];
			hands_[1].erase(i);
			for (int j = 0; j < 12; j++) {
				if (board_[j].animal == animal_t::E) {
#define P2() printf("prev=%d j=%d c=%d\n", prev, j, c)
					board_[j].animal = prev;
					board_[j].dir = true;
					std::pair<case_t, bool> id = board_id();
					int c = get_win_count(id.first);
					if (c < 0) {
						//P2();
						//print();
						//printf("id2=%d\n", id2.first);
						//from_board_id(id2.first, id2.second);
						//print();
						//getchar();
						return 1;
					}
					if (c == 0) {
						//P2();
						//print();
						//printf("id2=%d\n", id2.first);
						//getchar();
						//return 0;
						res = 0;
					}
					board_[j].animal = animal_t::E;
					board_[j].dir = false;
				}
			}
			hands_[1].insert(i, prev);
		}
		//printf("-1\n");
		return res;
	}

	std::pair<case_t, bool> next_board() {
		flipy();
		int res = -1;
		int win_c = 128;
		int loose_c = 0;
		std::pair<case_t, bool> res_id;
		for (int i = 0; i < 12; i++) {
			if (board_[i].dir) {
				if (board_[i].animal == animal_t::E || !board_[i].dir)
					continue;
				int x = i%3;
				int y = i/3;
				animal_t animal = board_[i].animal;
				const pos *moves = animal_moves(animal);
				for (; moves->x <= 1; moves++) {
					int y2 = y - moves->y;
					if (y2 < 0 || 4 <= y2)
						continue;
					int x2 = x + moves->x;
					if (x2 < 0 || 3 <= x2)
						continue;
					int k = y2*3+x2;
					cell_t orig = board_[k];
					if (orig.dir)
						continue;
					board_[k].animal = animal;
					board_[k].dir = true;
					if (y2 == 3 && animal == animal_t::P)
						board_[k].animal = animal_t::N;
					board_[i].animal = animal_t::E;
					board_[i].dir = false;
					if (orig.animal != animal_t::E) {
						animal_t ani = orig.animal;
						if (ani == animal_t::N)
							ani = animal_t::P;
						hands_[1].push_back(ani);
					}
					std::pair<case_t, bool> id = board_id();
					int c = get_win_count(id.first);
					if (c < 0) {
						if (-c < win_c) {
							win_c = -c;
							res_id = id;
							res = 1;
						}
					}
					if (c == 0) {
						if (res < 0) {
							res_id = id;
							res = 0;
						}
					}
					if (c > 0) {
						if (res < 0 && loose_c < c) {
							loose_c = c;
							res_id = id;
						}
					}
					board_[i].animal = animal;
					board_[i].dir = true;
					board_[k] = orig;
					if (orig.animal != animal_t::E)
						hands_[1].pop_back();
				}
			}
		}

		animal_t prev = animal_t::E;
		for (std::size_t i = 0; i < hands_[1].size(); i++) {
			if (prev == hands_[1][i])
				continue;
			prev = hands_[1][i];
			hands_[1].erase(i);
			for (int j = 0; j < 12; j++) {
				if (board_[j].animal == animal_t::E) {
#define P2() printf("prev=%d j=%d c=%d\n", prev, j, c)
					board_[j].animal = prev;
					board_[j].dir = true;
					std::pair<case_t, bool> id = board_id();
					int c = get_win_count(id.first);
					if (c < 0) {
						if (-c < win_c) {
							win_c = -c;
							res_id = id;
							res = 1;
						}
					}
					if (c == 0) {
						if (res < 0) {
							res_id = id;
							res = 0;
						}
					}
					if (c > 0) {
						if (res < 0 && loose_c < c) {
							loose_c = c;
							res_id = id;
						}
					}
					board_[j].animal = animal_t::E;
					board_[j].dir = false;
				}
			}
			hands_[1].insert(i, prev);
		}
		return res_id;
	}
};

void load_turn(int turn) {
	printf("loading\n");
	char filename[1000];
	sprintf(filename, "turn%d.dat", turn);
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("fopen(r) failed\n");
		exit(1);
	}
	fread(win_counts, 11878227*28, 1, fp);
	fclose(fp);
}

int simulate() {
	load_turn(78);

	std::pair<game::case_t, bool> id;
	id.first = 571544193;
	id.second = false;

	game g;
	g.from_board_id(id.first, id.second);
	g.print();
	getchar();

	int c;
	int turn = 1;
	while (true) {
		g.from_board_id(id.first, id.second);
		id = g.next_board();
		c = get_win_count(id.first);
		if (c == 1) {
			printf("player 2 wins.");
			return 0;
		}
		if (c == -1) {
			printf("player 1 wins.");
			return 0;
		}
		g.from_board_id(id.first, !id.second);
		g.flipy();
		printf("turn %d\n", turn++);
		g.print();
		getchar();

		g.from_board_id(id.first, id.second);
		id = g.next_board();
		if (c == 1) {
			printf("player 1 wins.");
			return 0;
		}
		if (c == -1) {
			printf("player 2 wins.");
			return 0;
		}
		g.from_board_id(id.first, !id.second);
		printf("turn %d\n", turn++);
		g.print();
		getchar();
	}
}

int js(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: ./t1.out js id\n");
		return 1;
	}
	int id = atoi(argv[1]);

#if defined(__linux__) || defined(__APPLE__)
	int fd = open("turn78.dat", O_RDONLY);
	if (fd == -1) {
		printf("open(r) failed\n");
		return 1;
	}
	win_counts = (char *)mmap(NULL, 11878227*28, PROT_READ, MAP_PRIVATE, fd, 0);
	if (win_counts == MAP_FAILED) {
		printf("mmap(r) failed\n");
		return 1;
	}
#else
#error "not implemented."
#endif

	game g;
	g.from_board_id(id, false);
	g.flipy();
	std::pair<game::case_t, bool> res = g.next_board();
	g.from_board_id(res.first, res.second);
	res = g.board_id();
	printf("[%d, %s]\n", res.first, (res.second ? "true" : "false"));
	return 0;
}

int main(int argc, char *argv[])
{
	int idx = 0;
	for (int i = 0; i < 54; i++) {
		if (skip_table[i])
			table_map[i] = -1;
		else
			table_map[i] = idx++;
	}

	if (strcmp(argv[1], "js") == 0) {
		return js(argc-1, argv+1);
	}

	if (argc == 2 && strcmp(argv[1], "sim") == 0) {
		win_counts = new char [11878227*28];
		return simulate();
	}

	game g;
	g.from_board_id(571544193, false);
	g.print();
	std::pair<game::case_t, bool> id = g.board_id();
	printf("%d %d\n", id.first, id.second);

#if false
	for (game::case_t i = 0; i < 100000000; i++) {
		g.from_board_id(i, false);
		std::pair<game::case_t, bool> id = g.board_id();
		if (id.first != i || id.second) {
			printf("%d != %d\n", id.first, i);
			g.print();
			getchar();
		}
	}
#endif

#if true
	if (argc < 3) {
		printf("usage: ./t1.out first_turn last_turn\n");
		return 1;
	}
	int first_turn = atoi(argv[1]);
	int last_turn = atoi(argv[2]);
	if (first_turn < 1 || 126 < last_turn || first_turn >= last_turn) {
		printf("invalid argument\n");
		return 1;
	}

	win_counts = new char [11878227*28];
	game::case_t n = 641424258;
	if (first_turn == 1) {
		int c1 = 0;
		int c2 = 0;
		for (game::case_t i = 0; i < n; i++) {
			if (skip_table[i/11878227]) {
				i += 11878227-1;
				continue;
			}
			if (i % 11878227 == 0)
				printf("i=%d j=%d\n", i/11878227, table_map[i/11878227]);
			g.from_board_id(i, false);
			int x = g.win_or_loose_leaf();
			if (x > 0)
				c1++;
			if (x < 0)
				c2++;
			win_counts[table_map[i/11878227]*11878227+i%11878227] = x;
		}
		first_turn++;
	} else {
		load_turn(first_turn-1);
	}

	char *win_counts2 = new char [11878227*28];
	for (int turn = first_turn; turn < last_turn; turn++) {
		printf("turn: %d\n", turn);
		memcpy(win_counts2, win_counts, 11878227*28);
		printf("memcpy done\n");

		int c1 = 0;
		int c2 = 0;
		for (game::case_t i = 0; i < n; i++) {
			if (skip_table[i/11878227]) {
				i += 11878227-1;
				continue;
			}
			if (i % 11878227 == 0)
				printf("i=%d j=%d\n", i/11878227, table_map[i/11878227]);
			int j = table_map[i/11878227]*11878227+i%11878227;
			if (win_counts[j] == 0) {
				g.from_board_id(i, false);
				int x = g.win_or_loose()*turn;
				win_counts2[j] = x;
				if (x > 0) {
					c1++;
					//g.from_board_id(i, false);
					//printf("i=%d, win\n", i);
					//g.print();
					//getchar();
				}
				if (x < 0) {
					c2++;
					//g.from_board_id(i, false);
					//printf("i=%d, loose\n", i);
					//g.print();
					//getchar();
				}
			}
		}
		printf("c1=%d\n", c1);
		printf("c2=%d\n", c2);
		printf("turn %d done.\n", turn);
		std::swap(win_counts, win_counts2);
		int x = 571544193;
		int j = table_map[x/11878227]*11878227+x%11878227;
		if (win_counts[j] != 0) {
			printf("turn=%d, x=%d, count=%d\n", turn, x, win_counts[j]);
			break;
		}
		x = 571558593;
		j = table_map[x/11878227]*11878227+x%11878227;
		if (win_counts[j] != 0) {
			printf("turn=%d, x=%d, count=%d\n", turn, x, win_counts[j]);
			break;
		}
	}
	printf("saving\n");
	char filename[1000];
	sprintf(filename, "turn%d.dat", last_turn-1);
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		printf("fopen(w) failed\n");
		return 1;
	}
	fwrite(win_counts, 11878227*28, 1, fp);
	fclose(fp);

#endif
}

/*
  全盤面数: 641424258
  １手勝ち: 458563742
  １手負け:  54204233
  ２手勝ち:  11921639
  ２手負け:  10970892
  ３手勝ち:  10304559
  ３手負け:   3854822
  ４手勝ち:   2745350
  ４手負け:   8172151
  ５手勝ち:   7379929
  ５手負け:   1448920
  ６手勝ち:   1321950
  ６手負け:   5151427
  ７手勝ち:   5790981
  ７手負け:    893388
  ８手勝ち:    945564
  ８手負け:   4076538
  ９手勝ち:   
  ９手負け:   
 */
