//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <cmath>
void DamagCalculator();

bool isJumping = false;
bool isSwordActive = false;
DWORD lastAttackTime = 0;
const DWORD ATTACK_COOLDOWN = 1500; // 1.5 секунды задержки
int swordDisplayFrames = 0;
const int SWORD_DISPLAY_DURATION = 10;

// секция данных игры  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed, jump, speedjump, vy;
    int direction;
    int hp;
    int hpMax;
    int damage;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
} sprite;

sprite Hero;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик
sprite skelet;
sprite babka;
sprite indikator;
sprite indikatorziro;
sprite hp;
sprite bable;
sprite sword; //меч
sprite bite; // укус

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

struct mouse {
    int x, y;
};

mouse Mouse;

void UpdateMouse()
{
    POINT p;
    GetCursorPos(&p);              // координаты экрана
    ScreenToClient(window.hWnd, &p); // координаты окна
    Mouse.x = p.x;
    Mouse.y = p.y;
}

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    //ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 
    Hero.hBitmap = (HBITMAP)LoadImageA(NULL, "Герой.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
   // enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    skelet.hBitmap = (HBITMAP)LoadImageA(NULL, "Враг.bmp",IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    babka.hBitmap = (HBITMAP)LoadImageA(NULL, "Бабка.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "Фон.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    indikator.hBitmap = (HBITMAP)LoadImageA(NULL, "Здоровьеок.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    indikatorziro.hBitmap = (HBITMAP)LoadImageA(NULL, "Здоровьенеок.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hp.hBitmap = (HBITMAP)LoadImageA(NULL, "ракушка.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    bable.hBitmap = (HBITMAP)LoadImageA(NULL, "пузырь.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sword.hBitmap = (HBITMAP)LoadImageA(NULL, "меч.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    bite.hBitmap = (HBITMAP)LoadImageA(NULL, "укус.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    //------------------------------------------------------

    Hero.width = 250;
    Hero.height = 300;
    Hero.speed = 30;//скорость перемещения ракетки
    Hero.jump = 100;
    Hero.speedjump = 0;
    Hero.vy = 0;
    Hero.x = window.width / 2.;//ракетка посередине окна
    Hero.y = window.height - Hero.height;//чуть выше низа экрана - на высоту ракетки
    Hero.hp = 3;
    Hero.hpMax = 3;
    Hero.damage = 1;

    skelet.width = 100;
    skelet.height = 100;
    skelet.speed = 10;
    skelet.direction = 1;
    skelet.x = window.width / 2;
    skelet.y = window.height / 2;
    skelet.hp = 2;
    skelet.hpMax = 2;
    skelet.damage = 1;

    babka.width = 50;
    babka.height = 50;
    babka.speed = 30;
    babka.x = 400;
    babka.y = 200;

    indikator.width = 300;
    indikator.height = 250;
    indikator.x = 50;
    indikator.y = 50;

    indikatorziro.width = 100;
    indikatorziro.height = 100;
    indikatorziro.x = 500;
    indikatorziro.y = 500;

    hp.width = 30;
    hp.height = 30;
    hp.x = 200;
    hp.y = 130;

    bable.width = 30;
    bable.height = 30;
    bable.x = 270;
    bable.y = 190;

    sword.width = 125;
    sword.height = 200;
    sword.x = Hero.x + Hero.width;
    sword.y = Hero.y;


    bite.width = 50;
    bite.height = 50;
    bite.x = skelet.x - bite.width;
    bite.y = skelet.y;

    enemy.x = Hero.x;//х координату оппонета ставим в ту же точку что и игрока

    ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 11;
    ball.rad = 20;
    ball.x = Hero.x;//x координата шарика - на середие ракетки
    ball.y = Hero.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;

   
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(0, 0, 255));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    //_itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    //TextOutA(window.context, 10, 10, "Score", 5);
    //TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(skelet.hp, txt, 10);
    TextOutA(window.context, 10, 300, "HP Enemy: ", 11);
    TextOutA(window.context, 300, 300, (LPCSTR)txt, strlen(txt));

    _itoa_s(Hero.hp, txt, 10);
    TextOutA(window.context, 400, 50, "HP HERO: ", 11);
    TextOutA(window.context, 650, 50, (LPCSTR)txt, strlen(txt));
}



void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, Hero.x, Hero.y, Hero.width, Hero.height, Hero.hBitmap);// ракетка игрока

    if (ball.dy < 0 && (enemy.x - Hero.width / 4 > ball.x || ball.x > enemy.x + Hero.width / 4))
    {
        //имитируем разумность оппонента. на самом деле, компьютер никогда не проигрывает, и мы не считаем попадает ли его ракетка по шарику
        //вместо этого, мы всегда делаем отскок от потолка, а раектку противника двигаем - подставляем под шарик
        //движение будет только если шарик летит вверх, и только если шарик по оси X выходит за пределы половины длины ракетки
        //в этом случае, мы смешиваем координаты ракетки и шарика в пропорции 9 к 1
        enemy.x = ball.x * .1 + enemy.x * .9;
    }

    ShowBitmap(window.context, enemy.x - Hero.width / 2, 0, Hero.width, Hero.height, enemy.hBitmap);//ракетка оппонента


    if (skelet.hp > 0) {
        ShowBitmap(window.context, skelet.x, skelet.y, skelet.width, skelet.height, skelet.hBitmap);
    }

    ShowBitmap(window.context, babka.x - babka.width / 2, babka.y - babka.height / 2, babka.width, babka.height, babka.hBitmap); //бабочка

    //ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик

    ShowBitmap(window.context, indikator.x, indikator.y, indikator.width, indikator.height, indikator.hBitmap); //Здоровье веселый

    //ShowBitmap(window.context, indikatorziro.x, indikatorziro.y, indikatorziro.width, indikatorziro.height, indikatorziro.hBitmap); //Здоровье грустный

    ShowBitmap(window.context, hp.x, hp.y, hp.width, hp.height, hp.hBitmap); //ракушки

    ShowBitmap(window.context, bable.x, bable.y, bable.width, bable.height, bable.hBitmap); //пузрь

    ShowBitmap(window.context, bite.x, bite.y, bite.width, bite.height, bite.hBitmap); //укус

    if (isSwordActive) {
        ShowBitmap(window.context, sword.x, sword.y, sword.width, sword.height, sword.hBitmap);
    }
    

}


void LimitRacket()
{
    Hero.x = max(Hero.x, 0);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    Hero.x = min(Hero.x, window.width - Hero.width);//аналогично для правого угла
}
void LimitFloor()
{
    Hero.y = max(Hero.y, 0);
    Hero.y = min(Hero.y, window.height - Hero.height - 10 );

   

}


void CheckWalls()
{
    
}

void CheckRoof()
{
 
}

bool tail = false;


void ProcessRoom()
{
    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
    CheckWalls();
    CheckRoof();
    //CheckFloor();
}

void ProcessBall()
{


}
void UpdatePhysics()
{
   float GRAVITY = 0; //скорость гравитации
   float GROUND_Y = window.height - Hero.height - 10;

    if (isJumping) 
    {
        Hero.vy += GRAVITY;
        Hero.y += Hero.vy;

        if (Hero.y >= GROUND_Y)
        {
            Hero.y = GROUND_Y;
            Hero.vy = 0;
            isJumping = false;
        }
       
    }
    else { //Гравитация в прыжке

        if (Hero.y < GROUND_Y)
        {
            Hero.vy += GRAVITY;
            Hero.y += Hero.vy;
        }
        else //
        {
            Hero.y = GROUND_Y;
            Hero.vy = 0;
        }
    }

    if (Hero.y < 0)
    {
        Hero.y = 0;
        Hero.vy = 0;
    }
}
void Jump()
{
    if (!isJumping)
    {
        Hero.vy = -Hero.jump; 
        isJumping = true;
        
    }
}

void UpdateSwordTimer()
{
    if (isSwordActive) {
        swordDisplayFrames--;
        if (swordDisplayFrames <= 0) {
            isSwordActive = false;
        }
    }
}
void ColisionEnemy(float x, float y)
{
    //if (x + sword.width <= skelet.x &&
    //    x >= skelet.x) {
    //
    //    DamagCalculator();
    //}
}
bool IsMouseOnSprite(int mx, int my, const sprite& s)
{
    return mx >= s.x && mx <= s.x + s.width &&
        my >= s.y && my <= s.y + s.height;
}

void ProcessInput()
{

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (IsMouseOnSprite(Mouse.x, Mouse.y, babka))
        {
            // клик по бабке
            ProcessSound("click.wav");
            babka.x += 20; // пример реакции
        }
    }

    if (GetAsyncKeyState(VK_RBUTTON)) {
        Hero.y = Mouse.y;
        Hero.x = Mouse.x;
    }

    if (GetAsyncKeyState(VK_RIGHT)) Hero.x += Hero.speed;

    if (GetAsyncKeyState(VK_SPACE) && !isJumping) {
        Jump(); //Прыжок на пробел
    }

    if (!game.action && GetAsyncKeyState(VK_SPACE))
    {
        game.action = true;
        ProcessSound("bounce.wav");
    }
    
    if (GetAsyncKeyState('C') & 0x8000) {
        DWORD currentTime = timeGetTime();

        if (currentTime - lastAttackTime >= ATTACK_COOLDOWN) {
            isSwordActive = true;
            swordDisplayFrames = SWORD_DISPLAY_DURATION;
            sword.x = Hero.x + Hero.width - 20;
            sword.y = Hero.y + 50;
            lastAttackTime = currentTime;
            //ProcessSound("sword.wav");
            DamagCalculator();
            
        }
    }
}





void MoveEnemy() {
    // Расчет расстояния до героя
    float dx = Hero.x - skelet.x;
    float dy = Hero.y - skelet.y;
    float distance = sqrt(dx * dx + dy * dy);

    const float ATTACK_DISTANCE = 150.0f; // расстояние на котором атакует
    const float FLEE_DISTANCE = 400.0f;  // расстояние на котором убегает

    if (skelet.hp > 1) {
        // Скелет преследует и атакует героя
        if (distance > ATTACK_DISTANCE) {
            // Преследование - движение к герою
            if (dx > 0) {
                skelet.x += skelet.speed;
            }
            else if (dx < 0) {
                skelet.x -= skelet.speed;
            }

            if (dy > 0) {
                skelet.y += skelet.speed;
            }
            else if (dy < 0) {
                skelet.y -= skelet.speed;
            }
        }
        else {
            // Атака - нанесение урона герою
            DWORD currentTime = timeGetTime();
            static DWORD lastEnemyAttackTime = 0;
            const DWORD ENEMY_ATTACK_COOLDOWN = 1000; // 1 секунда между атаками

            if (currentTime - lastEnemyAttackTime >= ENEMY_ATTACK_COOLDOWN) {
                Hero.hp -= skelet.damage;
                lastEnemyAttackTime = currentTime;

                // Показываем укус
                bite.x = skelet.x-bite.width;
                bite.y = skelet.y;

                // Звук атаки (раскомментируйте если есть файл)
                // ProcessSound("enemy_attack.wav");

                // Проверка смерти героя
                if (Hero.hp <= 0) {
                    Hero.hp = 0;
                    // Здесь можно добавить логику окончания игры
                }
            }
        }
    }
    else if (skelet.hp == 1) {
        // Скелет убегает от героя (при 1 HP)
        if (distance < FLEE_DISTANCE) {
            // Убегаем от героя - движение в противоположную сторону
            if (dx > 0) {
                skelet.x -= skelet.speed * 1.5f; // Убегаем быстрее
            }
            else if (dx < 0) {
                skelet.x += skelet.speed * 1.5f;
            }

            if (dy > 0) {
                skelet.y -= skelet.speed * 1.5f;
            }
            else if (dy < 0) {
                skelet.y += skelet.speed * 1.5f;
            }
        }
    }

    // Ограничение движения скелета в пределах экрана
    skelet.x = max(skelet.x, skelet.width / 2);
    skelet.x = min(skelet.x, window.width - skelet.width / 2);
    skelet.y = max(skelet.y, skelet.height / 2);
    skelet.y = min(skelet.y, window.height - skelet.height / 2);
}



void DamagCalculator() {

    if (isSwordActive && skelet.hp > 0) {
        sword.x = Hero.x + Hero.width - 20;
        sword.y = Hero.y + 50;

        if (sword.x < skelet.x + skelet.width / 2 &&
            sword.x + sword.width > skelet.x - skelet.width / 2 &&
            sword.y < skelet.y + skelet.height / 2 &&
            sword.y + sword.height > skelet.y - skelet.height / 2) {
            skelet.hp -= Hero.damage;

            if (skelet.hp < 0) {
                skelet.hp = 0;
            }
        }
        
    }
}
void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    mciSendString(TEXT("play ..\\Debug\\МузыкаФон.mp3 repeat"), NULL, 0, NULL);
    //ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        UpdateMouse();
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
        UpdatePhysics();
        UpdateSwordTimer();
        ProcessInput();//опрос клавиатуры
        
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран
        //LimitFloor();
        ProcessBall();//перемещаем шарик
        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
        MoveEnemy();
    }

}
