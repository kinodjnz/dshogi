$(document).ready(function() {
	var animal_names = {
		'L': 'lion',
		'K': 'kirin',
		'Z': 'zo',
		'P': 'piyo',
		'N': 'niwatori',
		' ': 'empty',
	};
	var image_url = function(ch, dir) {
		return 'url(' + animal_names[ch] + (dir&&ch!=' '?"2":"") + '.png)'
	};
	var disp = function(x,y,ch,dir) {
		var cell = $('table#board tr').eq(y).children('td').eq(x);
		cell.css('background', image_url(ch, dir));
		cell.css('border-color', 'white');
	};
	var width = 40;
	var height = 41;
	var board = [
		'Kv', 'Lv', 'Zv',
		'  ', 'Pv', '  ',
		'  ', 'P^', '  ',
		'Z^', 'L^', 'K^'
	];
	var hands = [];
	var hands2 = [];
	var disp_board = function(b) {
		for (var i = 0; i < 12; i++) {
			var x = i%3;
			var y = Math.floor(i/3);
			disp(x, y, b[i].charAt(0), b[i].charAt(1)=='v');
		}
		disp_board_id();
	};
	var disp_hands = function(hands, dir) {
		for (var i = 0; i < 8; i++) {
			var cell = $('table#hands' + (dir?'2':'') + ' tr td').eq(i);
			ch = (i >= hands.length ? ' ' : hands[i]);
			cell.css('background', image_url(ch, dir));
			cell.css('border-color', 'white');
		}
	};
	var place_hand = function(x, y, i, dir) {
		if (board[y*3+x].charAt(0)!=' ')
			return;
		if (dir) {
			var c = hands2[i] + 'v';
			hands2.splice(i, 1);
			board[y*3+x] = c;
		} else {
			var c = hands[i] + '^';
			hands.splice(i, 1);
			board[y*3+x] = c;
		}
	};
	var move = function(x, y, oldx, oldy) {
		if (x==oldx && y==oldy)
			return;
		var ch = board[oldy*3+oldx].charAt(0);
		if (ch == 'L') {
			if ((Math.abs(x-oldx) | Math.abs(y-oldy)) == 1)
				;
			else
				return;
		} else if (ch == 'K') {
			if ((Math.abs(x-oldx)==1 && y==oldy) ||
				(x==oldx && Math.abs(y-oldy)==1))
				;
			else
				return;
		} else if (ch == 'Z') {
			if ((x-oldx==1 && y-oldy==1) || (x-oldx==-1 && y-oldy==1) ||
				(x-oldx==1 && y-oldy==-1) || (x-oldx==-1 && y-oldy==-1))
				;
			else
				return;
		} else if (ch == 'P') {
			if (x==oldx && y-oldy==-1)
				;
			else
				return;
		} else if (ch == 'N') {
			if ((y-oldy==-1 && Math.abs(x-oldx)<=1) ||
				(y==oldy && Math.abs(x-oldx)==1) ||
				(y-oldy==1 && x==oldx))
				;
			else
				return;
		} else
			return;

		var dir = (board[oldy*3+oldx].charAt(1)=='v');
		if (dir)
			return;
		if (board[y*3+x].charAt(0) != ' ') {
			if (board[y*3+x].charAt(1)!='v')
				return;
			var ch = board[y*3+x].charAt(0);
			if (ch == 'N')
				ch = 'P';
			if (dir) {
				hands2.push(ch);
			} else {
				hands.push(ch);
			}
		}
		var new_ch = board[oldy*3+oldx];
		if (board[oldy*3+oldx].charAt(0) == 'P' &&
			(!dir && y == 0 || dir && y==3)) {
			new_ch = 'N' + (dir?'v':'^');
		}
		board[y*3+x] = new_ch;
		board[oldy*3+oldx] = '  ';
	}

	var captured = null;
	var oldx = null;
	var oldy = null;
	var bind_click = function(x, y) {
		var cell = $('table#board tr').eq(y).children('td').eq(x);
		cell.mousedown(function(ev) {
			var ex = ev.clientX;
			var ey = ev.clientY;
			if (captured == null) {
				if (board[y*3+x].charAt(0) == ' ') {
				    return;
				}
				if (board[y*3+x].charAt(1) == 'v') {
					return;
				}
			    captured = board[y*3+x];
				oldx = x;
				oldy = y;
				cell.css('border-color', 'red');
			} else {
				if (x != oldx || y != oldy) {
				    var got = board[y*3+x];
					if (oldy == -2) {
						place_hand(x, y, oldx, true);
					} else if (oldy == -1) {
						place_hand(x, y, oldx, false);
					} else {
						move(x, y, oldx, oldy);
					}
				}
				disp_board(board);
				disp_hands(hands, false);
				disp_hands(hands2, true);
				captured = null;
			}
		});
	}
	var bind_click_hand = function(i, dir) {
		var cell = $('table#hands' + (dir?'2':'') + ' tr td').eq(i);
		cell.mousedown(function(ev) {
			var ex = ev.clientX;
			var ey = ev.clientY;
			if (captured == null) {
				h = (dir ? hands2 : hands);
				if (h.length <= i || h[i] == ' ') {
				    return;
				}
			    captured = h[i] + (dir?'v':'^');
				oldx = i;
				oldy = (dir?-2:-1);
				cell.css('border-color', 'red');
			} else {
				if (oldx==i && oldy==(dir?-2:-1)) {
				    captured = null;
					cell.css('border-color', 'white');
				}
			}
		});
	};
	var idiv = function(x, y) {
		return Math.floor(x/y);
	};
	var tobool = function(b) {
		return (b?true:false);
	}
	var count_case = function(m, b) {
		//if (b == 1)
			return 2*m*m + 2*m + 3;
		//return 2*m*(m-1)*f(m-2, b-1) + 4*m*f(m-1, b-1) + 3*f(m, b-1);
	}
	var lesser = function(a, b) {
		return (a-b);
	}
	var board_id = function() {
		var h = {};
		var i;
		for (i = 0; i < 12; i++) {
			var c = board[i].charAt(0);
			var niwatori = false;
			if (c == 'N') {
				c = 'P';
				niwatori = true;
			}
			var dir = board[i].charAt(1)=='v';
			if (!(c in h))
				h[c] = [];
			h[c].push([0, dir, i%3, Math.floor(i/3), niwatori]);
		}
		for (i = 0; i < 8; i++) {
			if (!(hands[i] in h))
				h[hands[i]] = [];
			h[hands[i]].push([1, false]);
		}
		for (i = 0; i < 8; i++) {
			if (!(hands2[i] in h))
				h[hands2[i]] = [];
			h[hands2[i]].push([1, true]);
		}
		if (h['L'][0][1]) {
			var a = h['L'][0];
			h['L'][0] = h['L'][1];
			h['L'][1] = a;
		}
		var flipx = false;
		if (h['L'][0][2] == 2 || (h['L'][0][2] == 1 && h['L'][1][2] == 2)) {
			// flip
			flipx = true;
			for (var a in h) {
				for (var j = 0; j < h[a].length; j++) {
					if (h[a][j][0] == 0)
						h[a][j][2] = 2-h[a][j][2];
				}
			}
		}
		var encode_lion = function(lion1, lion2, ind) {
			var r;
			ind[0] = 3*lion1[3] + lion1[2];
			ind[1] = 3*lion2[3] + lion2[2];
			if (lion1[2] == 0) {
				if (ind[0] < ind[1])
					ind[1]--;
				r = (lion1[3]-1)*11 + ind[1];
			} else {
				var ind1 = 2*lion1[3] + lion1[2];
				var ind2 = 2*lion2[3] + lion2[2];
				if (ind1 < ind2)
					ind2--;
				r = 33 + (lion1[3]-1)*7 + ind2;
			}
			return r;
		};
		var update_ind = function(ani1, ani2, ind, j) {
			var swapped = false;
			ind1 = 3*ani1[3] + ani1[2];
			ind2 = 3*ani2[3] + ani2[2];
			if (ind1 > ind2) {
				swapped = true;
				var a = ind1;
				ind1 = ind2;
				ind2 = a;
			}
			for (var k = 0; k < j; k++) {
				if (ind[k] <= ind1) ind1--;
				if (ind[k] <= ind2) ind2--;
			}
			ind2--;
			ind[j] = ind1;
			ind[j+1] = ind2;
			return swapped;
		}
		var encode_hands = function(ani1, ani2) {
			var x;
			if (ani1[1] && ani2[1])
				x = 2;
			else if (!ani1[1] && !ani2[1])
				x = 0;
			else
				x = 1;
			return x;
		}
		var encode_zo = function(h, ind, m) {
			var zo1 = h['Z'][0];
			var zo2 = h['Z'][1];
			var r;
			var t = zo1[0] + zo2[0];
			if (t == 0) {
				if (update_ind(zo1, zo2, ind, 6)) {
					var a = zo1;
					zo1 = zo2;
					zo2 = a;
				}
				r = zo1[1]+zo2[1]*2;
				r = r + 4*((m*(m-1)/2-(m-1-ind[6])*(m-ind[6])/2) + (ind[7]-ind[6]));
			} else if (t == 1) {
				if (zo1[0] == 1) {
					var a = zo1;
					zo1 = zo2;
					zo2 = a;
				}
				ind[6] = 3*zo1[3] + zo1[2];
				ind[7] = 9999;
				for (var k = 0; k < 6; k++) {
					if (ind[k] <= ind[6]) ind[6]--;
				}
				r = zo1[1];
				r = r + 2*(ind[6] + m*zo2[1]);
				r = 2*m*(m-1) + r;
			} else {
				var x = encode_hands(zo1, zo2);
				r = 2*m*(m-1) + 4*m + x;
			}
			return r;
		}
		var encode_kirin = function(h, ind, m) {
			var kirin1 = h['K'][0];
			var kirin2 = h['K'][1];
			var r;
			var t = kirin1[0] + kirin2[0];
			if (t == 0) {
				if (update_ind(kirin1, kirin2, ind, 4)) {
					var a = kirin1;
					kirin1 = kirin2;
					kirin2 = a;
				}
				r = kirin1[1]+kirin2[1]*2;
				r = r + 4*((m*(m-1)/2-(m-1-ind[4])*(m-ind[4])/2) + (ind[5]-ind[4]));
				r = r + 2*m*(m-1)*encode_zo(h, ind, m-2);
			} else if (t == 1) {
				if (kirin1[0] == 1) {
					var a = kirin1;
					kirin1 = kirin2;
					kirin2 = a;
				}
				ind[4] = 3*kirin1[3] + kirin1[2];
				ind[5] = 9999;
				for (var k = 0; k < 4; k++) {
					if (ind[k] <= ind[4]) ind[4]--;
				}
				r = kirin1[1];
				r = r + 2*(ind[4] + m*kirin2[1]);
				r = 2*m*(m-1)*count_case(m-2, 1) + r;
				r += 4*m*encode_zo(h, ind, m-1);
			} else {
				var x = encode_hands(kirin1, kirin2);
				r = 2*m*(m-1)*count_case(m-2, 1) + 4*m*count_case(m-1, 1) + x;
				r += 3*encode_zo(h, ind, m);
			}
			return r;
		};
		var encode_piyo = function(h, ind) {
			var piyo1 = h['P'][0];
			var piyo2 = h['P'][1];
			var r;
			var t = piyo1[0] + piyo2[0];
			if (t == 0) {
				if (update_ind(piyo1, piyo2, ind, 2)) {
					var a = piyo1;
					piyo1 = piyo2;
					piyo2 = a;
				}
				r = piyo1[1]+piyo1[4]*2+piyo2[1]*4+piyo2[4]*8;
				r = r + 16*((45-(9-ind[2])*(10-ind[2])/2) + (ind[3]-ind[2]));
				r = r + 16*45*encode_kirin(h, ind, 8);
			} else if (t == 1) {
				if (piyo1[0] == 1) {
					var a = piyo1;
					piyo1 = piyo2;
					piyo2 = a;
				}
				ind[2] = 3*piyo1[3] + piyo1[2];
				ind[3] = 9999;
				if (ind[0] <= ind[2]) ind[2]--;
				if (ind[1] <= ind[2]) ind[2]--;
				r = piyo1[1]+piyo1[4]*2;
				r = r + 4*(ind[2] + 10*piyo2[1]);
				r = 13865 * (16*45) + r + 4*10*2*encode_kirin(h, ind, 9);
			} else {
				var x = encode_hands(piyo1, piyo2);
				r = 13865 * (16*45) + 22401 * (4*10*2) + x
				r += 3*encode_kirin(h, ind, 10);
			}
			return r;
		};
		var ind = new Array(8);
		for (var i = 0; i < ind.length; i++)
			ind[i] = 9999;
		var r_lion = encode_lion(h['L'][0], h['L'][1], ind);
		var r_piyo = encode_piyo(h, ind);
		var r = r_lion*11878227 + r_piyo;//54
		return [r, flipx];
	};
	var disp_board_id = function() {
		var a = board_id();
		$('#board_id').val(a[0]);
		if (a[1])
			$('#board_flipx').attr('checked', 'checked');
		else
			$('#board_flipx').removeAttr('checked');
	};

	var decode_lion = function(h, r, ind) {
		var a1 = [0, 0, 0, 0]
		var a2 = [0, 1, 0, 0];
		if (r < 33) {
			a1[2] = 0;
			a1[3] = idiv(r, 11)+1;
			var ind1 = r % 11;
			if (ind1 >= 3*a1[3]+a1[2])
				ind1++;
			a2[2] = ind1%3;
			a2[3] = idiv(ind1, 3);
		} else {
			r -= 33;
			a1[2] = 1;
			a1[3] = idiv(r, 7)+1;
			var ind1 = r % 7;
			if (ind1 >= 2*a1[3]+a1[2])
				ind1++;
			a2[2] = ind1%2;
			a2[3] = idiv(ind1, 2);
		}
		ind[0] = 3*a1[3] + a1[2];
		ind[1] = 3*a2[3] + a2[2];
		if (ind[0] < ind[1])
			ind[1]--;
		h['L'] = [a1, a2];
	};
	var decode_hands = function(h, r, ch) {
		if (r == 0)
			h[ch] = [[1, 0], [1, 0]];
		else if (r == 1)
			h[ch] = [[1, 0], [1, 1]];
		else if (r == 2)
			h[ch] = [[1, 1], [1, 1]];
	};
	var decode_ind = function(r, ind, j, m) {
		var ind1 = 0;
		var ind2 = 0;
		for (var s = m-1; s > 0; s--) {
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
		for (var i = j-1; i >= 0; i--) {
			if (ind[i] <= ind1) ind1++;
			if (ind[i] <= ind2) ind2++;
		}
		return [ind1, ind2];
	};
	var decode_zo = function(h, r, ind, m) {
		if (r < 2*m*(m-1)) {
			var a1 = [0, 0, 0, 0];
			var a2 = [0, 0, 0, 0];
			a1[1] = tobool(r&1);
			a2[1] = tobool(r&2);
			r = idiv(r, 4);
			var indr = decode_ind(r, ind, 6, m);
			a1[2] = indr[0]%3;
			a1[3] = idiv(indr[0], 3);
			a2[2] = indr[1]%3;
			a2[3] = idiv(indr[1], 3);
			h['Z'] = [a1, a2];
		} else if (r < 2*m*(m-1) + 4*m) {
			var a1 = [0, 0, 0, 0];
			var a2 = [1, 0];
			r -= 2*m*(m-1);
			a1[1] = tobool(r&1);
			r = idiv(r, 2);
			var ind1 = r%m;
			ind[6] = ind1;
			for (var i = 5; i >= 0; i--)
				if (ind[i] <= ind1) ind1++;
			a1[2] = ind1%3;
			a1[3] = idiv(ind1, 3);
			a2[1] = tobool(idiv(r, m));
			h['Z'] = [a1, a2];
		} else {
			r -= 2*m*(m-1) + 4*m;
			decode_hands(h, r, 'Z');
		}
	};
	var decode_kirin = function(h, r, ind, m) {
		var offset = 2*m*(m-1)*count_case(m-2, 1);
		if (r < offset) {
			var a1 = [0, 0, 0, 0];
			var a2 = [0, 0, 0, 0];
			var r_zo = idiv(r, 2*m*(m-1));
			r = r % (2*m*(m-1));
			a1[1] = tobool(r&1);
			a2[1] = tobool(r&2);
			r = idiv(r, 4);
			var indr = decode_ind(r, ind, 4, m);
			a1[2] = indr[0]%3;
			a1[3] = idiv(indr[0], 3);
			a2[2] = indr[1]%3;
			a2[3] = idiv(indr[1], 3);
			h['K'] = [a1, a2];
			decode_zo(h, r_zo, ind, m-2);
		} else {
			var offset2 = offset + 4*m*count_case(m-1, 1);
			if (r < offset2) {
				var a1 = [0, 0, 0, 0];
				var a2 = [1, 0];
				r -= offset;
				var r_zo = idiv(r, 4*m);
				r = r % (4*m);
				a1[1] = tobool(r&1);
				r = idiv(r, 2);
				var ind1 = r%m;
				ind[4] = ind1;
				for (var i = 3; i >= 0; i--)
					if (ind[i] <= ind1) ind1++;
				a1[2] = ind1%3;
				a1[3] = idiv(ind1, 3);
				a2[1] = tobool(idiv(r, m));
				h['K'] = [a1, a2];
				decode_zo(h, r_zo, ind, m-1);
			} else {
				r -= offset2;
				var r_zo = idiv(r, 3);
				r = r % 3;
				decode_hands(h, r, 'K');
				decode_zo(h, r_zo, ind, m);
			}
		}
	};
	var decode_piyo = function(h, r, ind) {
		if (r < 13865 * (16*45)) {
			var a1 = [0, 0, 0, 0, 0];
			var a2 = [0, 0, 0, 0, 0];
			var r_kirin = idiv(r, 16*45);
			r = r % (16*45);
			a1[1] = tobool(r&1);
			a1[4] = tobool(r&2);
			a2[1] = tobool(r&4);
			a2[4] = tobool(r&8);
			r = idiv(r, 16);
			var indr = decode_ind(r, ind, 2, 10);
			a1[2] = indr[0]%3;
			a1[3] = idiv(indr[0], 3);
			a2[2] = indr[1]%3;
			a2[3] = idiv(indr[1], 3);
			h['P'] = [a1, a2];
			decode_kirin(h, r_kirin, ind, 8);
		} else if (r < 13865 * (16*45) + 22401 * (4*10*2)) {
			var a1 = [0, 0, 0, 0, 0];
			var a2 = [1, 0];
			r -= 13865 * (16*45);
			var r_kirin = idiv(r, 4*10*2);
			r = r % (4*10*2);
			a1[1] = tobool(r&1);
			a1[4] = tobool(r&2);
			r = idiv(r, 4);
			var ind1 = r%10;
			ind[2] = ind1;
			for (var i = 1; i >= 0; i--)
				if (ind[i] <= ind1) ind1++;
			a1[2] = ind1%3;
			a1[3] = idiv(ind1, 3);
			a2[1] = tobool(idiv(r, 10));
			h['P'] = [a1, a2];
			decode_kirin(h, r_kirin, ind, 9);
		} else {
			r -= 13865 * (16*45) + 22401 * (4*10*2);
			var r_kirin = idiv(r, 3);
			r = r % 3;
			decode_hands(h, r, 'P');
			decode_kirin(h, r_kirin, ind, 10);
		}
	};
	var from_board_id = function(id, flipx) {
		var r_lion = idiv(id, 11878227); //id % 72;
		var r_piyo = id % 11878227; //idiv(id, 72);
		h = {};
		ind = new Array(8);
		for (var i = 0; i < ind.length; i++)
			ind[i] = 9999;
		decode_lion(h, r_lion, ind);
		decode_piyo(h, r_piyo, ind);
		var b = [
			'  ', '  ', '  ',
			'  ', '  ', '  ',
			'  ', '  ', '  ',
			'  ', '  ', '  '
		];
		var h1 = [];
		var h2 = [];
		var update = function(ani, ch) {
			var ind;
			if (flipx)
				ind = 3*ani[3] + (2-ani[2]);
			else
				ind = 3*ani[3] + ani[2];
			if (ani[0] == 0)
				b[ind] = ch + (ani[1]?'v':'^');
			else if (ani[1])
				h2.push(ch);
			else
				h1.push(ch);
		}
		update(h['L'][0], 'L');
		update(h['L'][1], 'L');
		update(h['P'][0], (h['P'][0][4]?'N':'P'));
		update(h['P'][1], (h['P'][1][4]?'N':'P'));
		update(h['K'][0], 'K');
		update(h['K'][1], 'K');
		update(h['Z'][0], 'Z');
		update(h['Z'][1], 'Z');
		return [b, h1, h2];
	};

	disp_board(board);
	for (var i = 0; i < 12; i++) {
		bind_click(i%3, Math.floor(i/3));
	}
	for (var i = 0; i < 8; i++) {
		bind_click_hand(i, false);
		//bind_click_hand(i, true);
	}
	$('#decode').click(function() {
		var r = from_board_id($('#board_id').val(),
							  tobool($('#board_flipx').attr('checked')))
		board = r[0];
		hands = r[1];
		hands2 = r[2];
		disp_board(board);
		disp_hands(hands, false);
		disp_hands(hands2, true);
	});
	$('#next').click(function() {
		var a = board_id();
		$.getJSON('/' + a[0], function(data) {
			var r = from_board_id(data[0], (a[1] ? !data[1] : data[1]));
			board = r[0];
			hands = r[1];
			hands2 = r[2];
			disp_board(board);
			disp_hands(hands, false);
			disp_hands(hands2, true);
		});
	});
	$('#test').click(function() {
		for (var i = 0; i < 5000; i++) {
			var z = Math.floor(Math.random() * 855232344);
			console.log('z='+z);
			var r = from_board_id(z, false);
			board = r[0];
			hands = r[1];
			hands2 = r[2];
			var x = board_id();
			if (z != x[0] || x[1])
				alert(z + ' ' + x);
		}
		disp_board(board);
		disp_hands(hands, false);
		disp_hands(hands2, true);
	});
});
