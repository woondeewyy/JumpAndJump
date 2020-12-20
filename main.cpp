//////////////////////////////////////
//
//	JumpAndJump
//	��һ��С��Ϸ
//	
//	huidong <mailkey@yeah.net>
//
//	����ʱ�䣺2020.11.27
//	����޸ģ�2020.12.20
//

#include <easyx.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <stdlib.h>

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// ������
const int m_nBlocksNum = 512;

// ����ɫ
COLORREF m_colorBk = RGB(0, 162, 232);

// �����༫ֵ
int m_nMinInterval = 50;
int m_nMaxInterval = 400;
// �����ȼ�ֵ
int m_nMinBlockSize = 20;
int m_nMaxBlockSize = 100;
// ���鶥���͵ײ�
int m_nBlockTop = WINDOW_HEIGHT - 100;
int m_nBlockBottom = WINDOW_HEIGHT;

// ��Ҵ�С��ֵ
int m_nMaxPlayerSize = 50;
int m_nMinPlayerSize = 20;

struct Player
{
	// ��������
	RECT rctBlocks[m_nBlocksNum] = { 0 };

	// ������ɫ
	COLORREF colorBlocks[m_nBlocksNum] = { 0 };

	// �������������黭�����棩
	IMAGE* imgBk = NULL;

	// ͼ��ƫ��
	int nImgOffsetX;

	// �÷�
	int nScore;

	// ������꣨�������½����꣩
	int nPlayerX, nPlayerY;

	// ��ǰ������
	float fPlayerSize;

	// �ɿ��ո����Ծ�ľ���
	float fJumpDistance;

	// ��ǰ���վ�ڵڼ���������
	int nStandBlockNum;

	// ���¿ո��־
	bool bEnter;

	// ������־
	bool bJump;
};


// ��Ծ���߼��㺯��
// x1, y1 ���κ�����x�����ཻ���
// top_x, top_y ���κ����Ķ���
// ���ع���x�Ķ��κ���������x��Ӧ������y
double jump_func(double x1, double y1, double top_x, double top_y, double x)
{
	/*
		��������ʽ��y = a(x - top_x)^2 + top_y
		a = (y1 - top_y) / (x1 - top_x)^2

		���� y = (y1 - top_y) / (x1 - top_x)^2 * (x - top_x)^2 + top_y
	*/

	return (y1 - top_y) / pow(x1 - top_x, 2) * pow(x - top_x, 2) + top_y;
}

// ��ȷ��ʱ����(���Ծ�ȷ�� 1ms������ ��1ms)
// by yangw80<yw80@qq.com>, 2011-5-4
void HpSleep(int ms)
{
	static clock_t oldclock = clock();		// ��̬��������¼��һ�� tick

	oldclock += ms * CLOCKS_PER_SEC / 1000;	// ���� tick

	if (clock() > oldclock)					// ����Ѿ���ʱ��������ʱ
		oldclock = clock();
	else
		while (clock() < oldclock)			// ��ʱ
			Sleep(1);						// �ͷ� CPU ����Ȩ������ CPU ռ����
}

void init_graph()
{
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
	setbkcolor(m_colorBk);
	setbkmode(TRANSPARENT);
	cleardevice();
	BeginBatchDraw();
	srand((UINT)time(NULL));
}

void end_graph()
{
	EndBatchDraw();
	closegraph();
}

void init_player(Player* player)
{
	player->nStandBlockNum = 0;
	player->fPlayerSize = (float)m_nMaxPlayerSize;
	player->nScore = 0;
	player->nStandBlockNum = 0;
	player->nImgOffsetX = 0;
	player->fJumpDistance = 0;
	player->bEnter = false;
	player->bJump = false;

	int last = 0;

	for (int i = 0; i < m_nBlocksNum; i++)
	{
		player->rctBlocks[i].top = m_nBlockTop;
		player->rctBlocks[i].bottom = m_nBlockBottom;
		player->rctBlocks[i].left = i == 0 ? player->nImgOffsetX : last + rand() % m_nMaxInterval + m_nMinInterval;
		player->rctBlocks[i].right = player->rctBlocks[i].left + rand() % m_nMaxBlockSize + m_nMinBlockSize;
		player->colorBlocks[i] = rand();
		last = player->rctBlocks[i].right;
	}

	if (!player->imgBk)
		player->imgBk = new IMAGE;
	player->imgBk->Resize(player->rctBlocks[m_nBlocksNum - 1].right, WINDOW_HEIGHT);
	SetWorkingImage(player->imgBk);

	setbkcolor(m_colorBk);
	cleardevice();
	for (int i = 0; i < m_nBlocksNum; i++)
	{
		setlinecolor(player->colorBlocks[i]);
		setfillcolor(player->colorBlocks[i]);
		fillrectangle(
			player->rctBlocks[i].left,
			player->rctBlocks[i].top,
			player->rctBlocks[i].right,
			player->rctBlocks[i].bottom
		);
	}

	SetWorkingImage();

	player->nPlayerX = player->rctBlocks[0].left;
	player->nPlayerY = m_nBlockTop;
}

void user_input(Player* player)
{
	if (KEY_DOWN(VK_SPACE))
	{
		player->bEnter = true;

		// ���С��û���¶׵������ߣ��ͼ����¶�
		if (player->fPlayerSize > m_nMinPlayerSize)
		{
			// С������׼��ʱ����߼�ֵ
			const float fJumpPrepareHeight = (float)0.03;

			// ��Ծ������ֵ��ͨ������ȷ��С�˵ĵ���������������һ������
			const float fJumpLength = (float)(m_nMaxInterval * 1.5 / (m_nMaxPlayerSize - m_nMinPlayerSize) * fJumpPrepareHeight);

			player->fPlayerSize -= fJumpPrepareHeight;
			player->fJumpDistance += fJumpLength;
		}
	}

	// �ɿ��ո񣬾�����
	else if (player->bEnter)
	{
		player->bEnter = false;
		player->bJump = true;
	}
}

void draw(Player* player)
{
	cleardevice();

	// ��������
	putimage(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, player->imgBk, player->nImgOffsetX, 0);

	// ��С��
	setlinecolor(BLACK);
	setfillcolor(BLUE);
	fillrectangle(
		player->nPlayerX - player->nImgOffsetX,			// ������Ҫ����ƫ�ƣ�С�˵�λ��Ҫ�ͻ���λ�ö�Ӧ��������Ҫ��ȥ����ƫ����
		player->nPlayerY - (int)player->fPlayerSize,	// �����Ƿ������½����꣬������Ҫ������������Ͻ�����
		(player->nPlayerX - player->nImgOffsetX) + m_nMaxPlayerSize,	// �Ҳ�x���������x���������ҿ��
		player->nPlayerY							// �����Ƿ������½����꣬���Եײ�y����ֱ����
	);

	// ������
	wchar_t strSocre[12] = { 0 };
	_itow_s(player->nScore, strSocre, 12, 10);

	settextcolor(WHITE);
	settextstyle(32, 0, L"system");
	outtextxy(30, 30, strSocre);

	FlushBatchDraw();
}

void lose_menu(Player* player)
{
	RECT rctTip = { 0, WINDOW_HEIGHT / 2 - 60, WINDOW_WIDTH, WINDOW_HEIGHT / 2 + 60 };

	setfillcolor(BLUE);
	fillrectangle(rctTip.left, rctTip.top, rctTip.right, rctTip.bottom);

	// ������
	wchar_t strSocre[12] = { 0 };
	wsprintf(strSocre, L"�÷֣�%d", player->nScore);

	settextstyle(48, 0, L"system");
	drawtext(strSocre, &rctTip, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

	rctTip.top = rctTip.bottom - 20;
	settextstyle(12, 0, L"system");
	drawtext(L"��Enter���¿�ʼ", &rctTip, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

	while (!KEY_DOWN(VK_RETURN))
	{
		FlushBatchDraw();
		Sleep(10);
	}
}

void game_run(Player* player)
{
	draw(player);

	// ����
	if (player->bJump)
	{
		player->bJump = false;

		// �ָ����
		for (; player->fPlayerSize < m_nMaxPlayerSize; player->fPlayerSize++)
		{
			draw(player);
			Sleep(6);
		}

		// ����xy�����仯�����ڴ�����
		int x = player->nPlayerX;
		int y = player->nPlayerY;

		// �Ƿ��Ѿ�����
		bool bJumped = false;

		// ��Ծ
		for (; ; player->nPlayerX++)
		{
			// ��Ծ���߼���
			player->nPlayerY =
				(int)jump_func(
					x,									// ���κ����к�x�ύ�������x����
					y,									// ���κ����к�x�ύ�������y����
					x + player->fJumpDistance / 2,		// ���κ����ж���x���꣨��x�ύ���������֮����߶ε�һ�룩
					y - player->fJumpDistance / 3,		// ���κ����ж���y���꣨ȡx�ύ���������֮����߶ε�1/3��Ϊ��Ծ�߶ȣ�
					player->nPlayerX					// ����x�����Ӧ��y����
				);

			// y����ˣ�������Ѿ�����
			if (player->nPlayerY < y)
				bJumped = true;

			draw(player);
			HpSleep(1);

			// �Ѿ����������䵽ԭ����y��
			if (player->nPlayerY >= y && bJumped)
			{
				// �ж��Ƿ���Ծ���˵���
				for (int i = player->nStandBlockNum; i < m_nBlocksNum; i++)
				{
					// �ж����﷽������½ǵ�����½ǵ��Ƿ���������
					if (
						((player->nPlayerX > player->rctBlocks[i].left &&
							player->nPlayerX < player->rctBlocks[i].right) ||
							(player->nPlayerX + m_nMaxPlayerSize > player->rctBlocks[i].left &&
								player->nPlayerX + m_nMaxPlayerSize < player->rctBlocks[i].right)) &&
						player->nPlayerY - y < 10 /* ����ĳ�����ʾ��ʱ�ж����ȣ���y�����ڶ��ķ�Χ����Ȼ�����ж����ɱ��⴩ģ���� */
						)
					{
						// վ���Ҳ���ԭ�����ϣ��ӷ�
						if (i != player->nStandBlockNum)
						{
							player->nScore++;
							player->nStandBlockNum = i;
						}

						// ������������ԭ���飬����˾�����ѭ������������վ�ڷ�����ȴ������ȥ��bug
						goto out;
					}
				}
			}

			// �Ѿ�������Ļ��
			if (player->nPlayerY >= WINDOW_HEIGHT && player->nPlayerX > (x + player->fJumpDistance) / 2)
			{
				lose_menu(player);
				init_player(player);
				return;
			}
		}
	out:
		// y���긴ԭ
		player->nPlayerY = y;

		// ��Ծ��������
		player->fJumpDistance = 0;

		Sleep(100);

		// �����ƶ�
		for (; player->nImgOffsetX <= player->nPlayerX - 20; player->nImgOffsetX++)
		{
			draw(player);
		}
	}

}

void startmenu(Player* player)
{
	draw(player);

	settextstyle(72, 0, L"����");
	settextcolor(BLUE);
	for (int x = 220, y = 120; x > 215, y > 115; x--, y--)
	{
		outtextxy(x, y, L"��һ��");
		FlushBatchDraw();
		Sleep(150);
	}

	Sleep(100);
	settextstyle(16, 0, L"system");
	outtextxy(230, 220, L"made by huidong 2020.12.20");
	FlushBatchDraw();

	Sleep(800);
	settextcolor(WHITE);
	outtextxy(140, 400, L" <<< Press and hold the space to start your first jump >>> ");

	while (true)
	{
		FlushBatchDraw();
		Sleep(10);
	
		if (_kbhit())
		{
			if (_getch() == ' ')
			{
				break;
			}
		}
	}
}

int main()
{
	Player player;

	init_graph();
	init_player(&player);
	startmenu(&player);

	while (true)
	{
		user_input(&player);
		game_run(&player);
	}

	end_graph();
	return 0;
}
