#include "generator.hpp"

#include <vector>
#include <thread>
#include <string>

#include "primitives.hpp"
#include "outputs.hpp"
#include "spec.hpp"
#include "pool.hpp"
#include "build.hpp"
#include "utils.hpp"
#include "dict.hpp"
#include "task.hpp"
#include "output_queue.hpp"

#include <memory>
#include <iostream>
#include <type_traits>

size_t g_chunk_size = 16*1024;
constexpr size_t g_vector_size = 1024;

#define R __restrict__

#define ALIGN __attribute__((aligned(64)))
#define HAS_ALIGN

struct VData {
	ALIGN size_t len[g_vector_size];
	ALIGN size_t pos[g_vector_size];

	int64_t a[g_vector_size];
	std::vector<char> buf;
	char* s[g_vector_size];

	bool tmp_pred[g_vector_size];
	int tmp_sel[g_vector_size];
	int tmp_sel2[g_vector_size];

	int64_t tmp_vals[g_vector_size];

	BaseType res_type = I64;

	int64_t* res;
};


struct WorkerState {
	std::vector<VData> cols;
	std::vector<size_t> tmp;
	std::vector<size_t*> tmp2;
};

thread_local WorkerState state;

NO_INLINE void
calc_positions_wide_prepare(size_t num, size_t num_cols,
	VData* R cols, size_t* R tmp, size_t** R tmp2)
{
	#define DEF(i) \
		const size_t* R len##i = &cols[c+i].len[0]; \
		size_t* R pos##i = &cols[c+i].pos[0];
	#define WRA(o) tmp[o + c + i*num_cols] = len##o[i];
	#define WRB(o) tmp2[o + c + i*num_cols] = (size_t*)&pos##o[i];

	size_t c = 0;

	// unroll for 8 cols
	for (; c+8 < num_cols; c+=8) {
		DEF(0); DEF(1); DEF(2); DEF(3); DEF(4); DEF(5); DEF(6); DEF(7);
		for (size_t i=0; i<num; i++) {
			WRA(0); WRA(1); WRA(2); WRA(3); WRA(4); WRA(5); WRA(6); WRA(7);
			WRB(0); WRB(1); WRB(2); WRB(3); WRB(4); WRB(5); WRB(6); WRB(7);
		}
	}

	// unroll for 2 cols
	for (; c+2 < num_cols; c+=2) {
		DEF(0); DEF(1);
		for (size_t i=0; i<num; i++) {
			WRA(0); WRA(1);
			WRB(0); WRB(1);
		}
	}

	while (c < num_cols) {
		DEF(0);
		for (size_t i=0; i<num; i++) {
			WRA(0);
			WRB(0);
		}
		c++;
	}
	#undef WRA
	#undef WRB
	#undef DEF

}

template<size_t num_cols>
NO_INLINE size_t
tcalc_positions_wide_prefix(size_t pos, size_t num,
	size_t* R tmp, size_t** R tmp2, size_t len_sep,
	size_t len_nl, VData* R cols)
{
#define KERNEL()
	size_t* R tmp_rd = tmp; \
	size_t** R tmp2_rd = tmp2; \
	size_t idx = 0; \
	for (size_t i=0; i<num; i++) { \
		size_t c; \
		for (c = 0; c < num_cols-1; c++) { \
			*(tmp2_rd[idx]) = pos; \
			pos += tmp_rd[idx]; \
			pos += len_sep; \
			idx++; \
		} \
		c = num_cols-1; \
		*(tmp2_rd[idx]) = pos; \
		pos += tmp_rd[idx]; \
		pos += len_nl; \
		idx++; \
	}

	KERNEL();
	return pos;
}

NO_INLINE size_t
calc_positions_wide_prefix(size_t pos, size_t num, size_t num_cols,
	size_t* R tmp, size_t** R tmp2, size_t len_sep, size_t len_nl, VData* R cols)
{
	calc_positions_wide_prepare(num, num_cols, cols, tmp, tmp2);

	switch (num_cols) {
	#define A(N) case N: return tcalc_positions_wide_prefix<N>(pos, num, tmp, tmp2, len_sep, len_nl, cols)

	// generated from haskell: foldl (++) "" $ map (\x -> "A(" ++ show x  ++ ");") [1..1024]
	A(1);A(2);A(3);A(4);A(5);A(6);A(7);A(8);A(9);A(10);A(11);A(12);A(13);A(14);A(15);A(16);A(17);A(18);A(19);A(20);A(21);A(22);A(23);A(24);A(25);A(26);A(27);A(28);A(29);A(30);A(31);A(32);A(33);A(34);A(35);A(36);A(37);A(38);A(39);A(40);A(41);A(42);A(43);A(44);A(45);A(46);A(47);A(48);A(49);A(50);A(51);A(52);A(53);A(54);A(55);A(56);A(57);A(58);A(59);A(60);A(61);A(62);A(63);A(64);A(65);A(66);A(67);A(68);A(69);A(70);A(71);A(72);A(73);A(74);A(75);A(76);A(77);A(78);A(79);A(80);A(81);A(82);A(83);A(84);A(85);A(86);A(87);A(88);A(89);A(90);A(91);A(92);A(93);A(94);A(95);A(96);A(97);A(98);A(99);A(100);A(101);A(102);A(103);A(104);A(105);A(106);A(107);A(108);A(109);A(110);A(111);A(112);A(113);A(114);A(115);A(116);A(117);A(118);A(119);A(120);A(121);A(122);A(123);A(124);A(125);A(126);A(127);A(128);A(129);A(130);A(131);A(132);A(133);A(134);A(135);A(136);A(137);A(138);A(139);A(140);A(141);A(142);A(143);A(144);A(145);A(146);A(147);A(148);A(149);A(150);A(151);A(152);A(153);A(154);A(155);A(156);A(157);A(158);A(159);A(160);A(161);A(162);A(163);A(164);A(165);A(166);A(167);A(168);A(169);A(170);A(171);A(172);A(173);A(174);A(175);A(176);A(177);A(178);A(179);A(180);A(181);A(182);A(183);A(184);A(185);A(186);A(187);A(188);A(189);A(190);A(191);A(192);A(193);A(194);A(195);A(196);A(197);A(198);A(199);A(200);A(201);A(202);A(203);A(204);A(205);A(206);A(207);A(208);A(209);A(210);A(211);A(212);A(213);A(214);A(215);A(216);A(217);A(218);A(219);A(220);A(221);A(222);A(223);A(224);A(225);A(226);A(227);A(228);A(229);A(230);A(231);A(232);A(233);A(234);A(235);A(236);A(237);A(238);A(239);A(240);A(241);A(242);A(243);A(244);A(245);A(246);A(247);A(248);A(249);A(250);A(251);A(252);A(253);A(254);A(255);A(256);A(257);A(258);A(259);A(260);A(261);A(262);A(263);A(264);A(265);A(266);A(267);A(268);A(269);A(270);A(271);A(272);A(273);A(274);A(275);A(276);A(277);A(278);A(279);A(280);A(281);A(282);A(283);A(284);A(285);A(286);A(287);A(288);A(289);A(290);A(291);A(292);A(293);A(294);A(295);A(296);A(297);A(298);A(299);A(300);A(301);A(302);A(303);A(304);A(305);A(306);A(307);A(308);A(309);A(310);A(311);A(312);A(313);A(314);A(315);A(316);A(317);A(318);A(319);A(320);A(321);A(322);A(323);A(324);A(325);A(326);A(327);A(328);A(329);A(330);A(331);A(332);A(333);A(334);A(335);A(336);A(337);A(338);A(339);A(340);A(341);A(342);A(343);A(344);A(345);A(346);A(347);A(348);A(349);A(350);A(351);A(352);A(353);A(354);A(355);A(356);A(357);A(358);A(359);A(360);A(361);A(362);A(363);A(364);A(365);A(366);A(367);A(368);A(369);A(370);A(371);A(372);A(373);A(374);A(375);A(376);A(377);A(378);A(379);A(380);A(381);A(382);A(383);A(384);A(385);A(386);A(387);A(388);A(389);A(390);A(391);A(392);A(393);A(394);A(395);A(396);A(397);A(398);A(399);A(400);A(401);A(402);A(403);A(404);A(405);A(406);A(407);A(408);A(409);A(410);A(411);A(412);A(413);A(414);A(415);A(416);A(417);A(418);A(419);A(420);A(421);A(422);A(423);A(424);A(425);A(426);A(427);A(428);A(429);A(430);A(431);A(432);A(433);A(434);A(435);A(436);A(437);A(438);A(439);A(440);A(441);A(442);A(443);A(444);A(445);A(446);A(447);A(448);A(449);A(450);A(451);A(452);A(453);A(454);A(455);A(456);A(457);A(458);A(459);A(460);A(461);A(462);A(463);A(464);A(465);A(466);A(467);A(468);A(469);A(470);A(471);A(472);A(473);A(474);A(475);A(476);A(477);A(478);A(479);A(480);A(481);A(482);A(483);A(484);A(485);A(486);A(487);A(488);A(489);A(490);A(491);A(492);A(493);A(494);A(495);A(496);A(497);A(498);A(499);A(500);A(501);A(502);A(503);A(504);A(505);A(506);A(507);A(508);A(509);A(510);A(511);A(512);A(513);A(514);A(515);A(516);A(517);A(518);A(519);A(520);A(521);A(522);A(523);A(524);A(525);A(526);A(527);A(528);A(529);A(530);A(531);A(532);A(533);A(534);A(535);A(536);A(537);A(538);A(539);A(540);A(541);A(542);A(543);A(544);A(545);A(546);A(547);A(548);A(549);A(550);A(551);A(552);A(553);A(554);A(555);A(556);A(557);A(558);A(559);A(560);A(561);A(562);A(563);A(564);A(565);A(566);A(567);A(568);A(569);A(570);A(571);A(572);A(573);A(574);A(575);A(576);A(577);A(578);A(579);A(580);A(581);A(582);A(583);A(584);A(585);A(586);A(587);A(588);A(589);A(590);A(591);A(592);A(593);A(594);A(595);A(596);A(597);A(598);A(599);A(600);A(601);A(602);A(603);A(604);A(605);A(606);A(607);A(608);A(609);A(610);A(611);A(612);A(613);A(614);A(615);A(616);A(617);A(618);A(619);A(620);A(621);A(622);A(623);A(624);A(625);A(626);A(627);A(628);A(629);A(630);A(631);A(632);A(633);A(634);A(635);A(636);A(637);A(638);A(639);A(640);A(641);A(642);A(643);A(644);A(645);A(646);A(647);A(648);A(649);A(650);A(651);A(652);A(653);A(654);A(655);A(656);A(657);A(658);A(659);A(660);A(661);A(662);A(663);A(664);A(665);A(666);A(667);A(668);A(669);A(670);A(671);A(672);A(673);A(674);A(675);A(676);A(677);A(678);A(679);A(680);A(681);A(682);A(683);A(684);A(685);A(686);A(687);A(688);A(689);A(690);A(691);A(692);A(693);A(694);A(695);A(696);A(697);A(698);A(699);A(700);A(701);A(702);A(703);A(704);A(705);A(706);A(707);A(708);A(709);A(710);A(711);A(712);A(713);A(714);A(715);A(716);A(717);A(718);A(719);A(720);A(721);A(722);A(723);A(724);A(725);A(726);A(727);A(728);A(729);A(730);A(731);A(732);A(733);A(734);A(735);A(736);A(737);A(738);A(739);A(740);A(741);A(742);A(743);A(744);A(745);A(746);A(747);A(748);A(749);A(750);A(751);A(752);A(753);A(754);A(755);A(756);A(757);A(758);A(759);A(760);A(761);A(762);A(763);A(764);A(765);A(766);A(767);A(768);A(769);A(770);A(771);A(772);A(773);A(774);A(775);A(776);A(777);A(778);A(779);A(780);A(781);A(782);A(783);A(784);A(785);A(786);A(787);A(788);A(789);A(790);A(791);A(792);A(793);A(794);A(795);A(796);A(797);A(798);A(799);A(800);A(801);A(802);A(803);A(804);A(805);A(806);A(807);A(808);A(809);A(810);A(811);A(812);A(813);A(814);A(815);A(816);A(817);A(818);A(819);A(820);A(821);A(822);A(823);A(824);A(825);A(826);A(827);A(828);A(829);A(830);A(831);A(832);A(833);A(834);A(835);A(836);A(837);A(838);A(839);A(840);A(841);A(842);A(843);A(844);A(845);A(846);A(847);A(848);A(849);A(850);A(851);A(852);A(853);A(854);A(855);A(856);A(857);A(858);A(859);A(860);A(861);A(862);A(863);A(864);A(865);A(866);A(867);A(868);A(869);A(870);A(871);A(872);A(873);A(874);A(875);A(876);A(877);A(878);A(879);A(880);A(881);A(882);A(883);A(884);A(885);A(886);A(887);A(888);A(889);A(890);A(891);A(892);A(893);A(894);A(895);A(896);A(897);A(898);A(899);A(900);A(901);A(902);A(903);A(904);A(905);A(906);A(907);A(908);A(909);A(910);A(911);A(912);A(913);A(914);A(915);A(916);A(917);A(918);A(919);A(920);A(921);A(922);A(923);A(924);A(925);A(926);A(927);A(928);A(929);A(930);A(931);A(932);A(933);A(934);A(935);A(936);A(937);A(938);A(939);A(940);A(941);A(942);A(943);A(944);A(945);A(946);A(947);A(948);A(949);A(950);A(951);A(952);A(953);A(954);A(955);A(956);A(957);A(958);A(959);A(960);A(961);A(962);A(963);A(964);A(965);A(966);A(967);A(968);A(969);A(970);A(971);A(972);A(973);A(974);A(975);A(976);A(977);A(978);A(979);A(980);A(981);A(982);A(983);A(984);A(985);A(986);A(987);A(988);A(989);A(990);A(991);A(992);A(993);A(994);A(995);A(996);A(997);A(998);A(999);A(1000);A(1001);A(1002);A(1003);A(1004);A(1005);A(1006);A(1007);A(1008);A(1009);A(1010);A(1011);A(1012);A(1013);A(1014);A(1015);A(1016);A(1017);A(1018);A(1019);A(1020);A(1021);A(1022);A(1023);A(1024);

	#undef A

	default:
		KERNEL();
		return pos;
	}
#undef KERNEL
}

NO_INLINE size_t
calc_positions(size_t pos, size_t num, size_t num_cols,
	VData* R cols, size_t len_sep, size_t len_nl)
{
#define KERNEL(LAST_COL, MANY_COLS)  { \
			cols[c].pos[i] = pos; \
			pos += cols[c].len[i]; \
			if (LAST_COL) { \
				pos += len_nl; \
			} else { \
				pos += len_sep; \
			} \
		}


	switch (num_cols) {
	case 0:
		assert(false);
		break;
	case 1:
		for (size_t i=0; i<num; i++) {
			size_t c = 0;
			KERNEL(true, false);
		}
		break;

	default:
		size_t* R tmp = &state.tmp[0];
		size_t** R tmp2 = &state.tmp2[0];
		return calc_positions_wide_prefix(pos, num,
			num_cols, tmp, tmp2, len_sep, len_nl, cols);
	}
	
#undef KERNEL
	return pos;
}

constexpr size_t num_chars_int(int64_t x) {
	size_t r = 0;
	if (x < 0) {
		r++;
	}

	do {
		r++;
	} while (x /= 10);

	r++; // terminator

	return r;
}

struct OutputQueue;

struct DoTask {
	void operator()(Task&& t);
	void append_vector(StrBuffer& out, size_t start, size_t num, const RelSpec& rel);
};

static void
to_str(const ColSpec& col, size_t colid, size_t num)
{
	auto& scol = state.cols[colid];
	int64_t* a = &scol.res[0];
	char** s = &scol.s[0];

	col.ctype.match(
		[&] (Integer cint) {
			auto& buf = scol.buf;
			if (cint.max > 4611686018427387905ll || cint.min < -4611686018427387905ll) {
				throw std::invalid_argument("Integer ranges too high");
			}

			size_t max_chars = std::max(num_chars_int(cint.max), num_chars_int(cint.min));

			// null terminator
			max_chars++;

			// allocate
			if (buf.size() < num*max_chars) {
				buf.resize(num*max_chars);
			}

			fix_ptrs(s, num, max_chars, &buf[0]);

			str_int(s, (size_t*)&scol.len[0], a, num, &scol.tmp_vals[0],
				&scol.tmp_pred[0], &scol.tmp_sel[0], &scol.tmp_sel2[0],
				scol.res_type);
		},
		[] (String cstr) {
			// already a string
		}
	);
}

static void
gen_col(const ColType& ctype, const ColSpec& col, size_t colid, size_t start,
	size_t num, bool top_level)
{
	auto& scol = state.cols[colid];
	int64_t* a = &scol.a[0];
	char** s = &scol.s[0];

	scol.res = nullptr;

	ctype.match(
		[&] (Integer cint) {
			scol.res_type = GetFittingType(cint.min, cint.max);

			cint.cgen.match(
				[&] (Sequential& gseq) {
					gen_seq(a, num, start, cint.min, cint.max, scol.res_type);
				},
				[&] (Uniform& guni) {
					gen_uni(a, num, start, cint.min, cint.max, scol.res_type);
				},
				[&] (Poisson& gpoisson) {
					gen_poisson(a, num, start, cint.min, cint.max, gpoisson.mean, scol.res_type);
				}
			);

			scol.res = a;
		},
		[&] (String cstr)  {
			gen_col(cstr.index, col, colid, start, num, false);

			assert(cstr.dict);
			assert(scol.res);

			cstr.dict->Lookup(s, &scol.len[0], (size_t*)scol.res, num, nullptr, scol.res_type);
		}
	);

	if (top_level) {
		to_str(col, colid, num);
	}
}


NO_INLINE void
DoTask::append_vector(StrBuffer& out, size_t start, size_t num, const RelSpec& rel)
{
	size_t num_cols = rel.cols.size();
	{
		if (state.cols.size() < num_cols) {
			state.cols.resize(num_cols);
		}

		const size_t tmp_size = num_cols * g_vector_size;
		if (state.tmp.size() != tmp_size) {
			state.tmp.resize(tmp_size);
		}

		if (state.tmp2.size() != tmp_size) {
			state.tmp2.resize(tmp_size);
		}
	}

	assert(num_cols > 0);

	for (size_t c = 0; c < num_cols; c++) {
		gen_col(rel.cols[c].ctype, rel.cols[c], c, start, num, true);
	}

	// calculate positions
	size_t size = calc_positions(out.used, num, num_cols, &state.cols[0],
		rel.GetSepLen(false), rel.GetSepLen(true));

	if (out.capacity() < size) {
		out.resize(size);
	}

	out.used = size;

	char* dst = (char*)out.pointer();

	// copy strings to final location
	for (size_t c = 0; c < num_cols; c++) {
		auto& scol = state.cols[c];
		const bool last_col = c == num_cols-1;

		const char* sep = rel.GetSep(last_col);
		const size_t sep_len = rel.GetSepLen(last_col);

		scatter_out(dst, &scol.pos[0], &scol.s[0], &scol.len[0], sep, sep_len, num);
	}
}

NO_INLINE void
DoTask::operator()(Task&& t)
{
	// generate [start, end) from relation
	assert(t.rel);
	assert(t.end <= t.rel->card);

	size_t off = t.start;

	StrBuffer final;
	final.init(1024*1024*10);

	while (off < t.end) {
		size_t num = std::min(g_vector_size, t.end - off);

		assert(t.rel);

		append_vector(final, off, num, *t.rel);

		off += num;
	}

	(*t.outp)(t, std::move(final));
}

NO_INLINE void
generate(RelSpec& spec, Output& out)
{
	assert(spec.threads > 0);
	assert(g_chunk_size > 0);
	assert(g_vector_size > 0);

	if (spec.threads <= 0) {
		spec.threads = std::thread::hardware_concurrency();
	}
	if (spec.card <= 0) {
		return;
	}

	std::vector<std::unique_ptr<Dictionary>> objs;

	// create dictionaries
	for (auto& col : spec.cols) {
		col.ctype.match(
			[&] (String& cstr) {
				if (!cstr.dict) {
					cstr.dict = new FileDictionary(cstr.fname);
				}
				objs.emplace_back(std::unique_ptr<Dictionary>(cstr.dict));
			},
			[] (Integer cint) {}
		);
	}

	auto num_threads = std::min(spec.threads, (spec.card / (g_chunk_size + g_chunk_size - 1)));
	if (num_threads <= 0) {
		num_threads = 1;
	}

	size_t todo = spec.card;
	size_t offset = 0;
	size_t chunkId = 0;
	size_t num_chunks = (todo + g_chunk_size - 1) / g_chunk_size;

	auto output_queue = std::make_unique<OutputQueue>(num_chunks, num_threads, out);
	ThreadPool<Task, DoTask> g_pool(num_threads);

	while (todo > 0) {
		const size_t num = std::min(g_chunk_size, todo);

		g_pool.Push(Task {offset, offset + num, &spec, output_queue.get(), chunkId});

		chunkId++;
		todo -= num;
		offset += num;
	};

	assert(num_chunks == chunkId);
}