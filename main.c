#include "raylib.h"

#define ROWS 10
#define COLS 10
#define CELL_SIZE 50

#define PANEL_WIDTH 300

// Exploration animation speed
#define ANIMATION_DELAY 0.15f

// Robot movement speed
#define ROBOT_MOVE_DELAY 0.25f

typedef struct
{
    int row;
    int col;
} Node;

/* =========================================================
   GRID
   ========================================================= */

int grid[ROWS][COLS] = {0};

int startRow = 0;
int startCol = 0;

int goalRow = 9;
int goalCol = 9;

/* =========================================================
   BFS DATA
   ========================================================= */

Node bfsQueue[ROWS * COLS];

int bfsFront = 0;
int bfsRear = 0;

int bfsVisited[ROWS][COLS] = {0};
int bfsPath[ROWS][COLS] = {0};

Node bfsParent[ROWS][COLS];

int bfsAnimating = 0;
int bfsFinished = 0;

int bfsNodesExplored = 0;
int bfsPathLengthValue = 0;

/* =========================================================
   DFS DATA
   ========================================================= */

Node dfsStack[ROWS * COLS];

int dfsTop = 0;

int dfsVisited[ROWS][COLS] = {0};
int dfsPath[ROWS][COLS] = {0};

Node dfsParent[ROWS][COLS];

int dfsAnimating = 0;
int dfsFinished = 0;

int dfsNodesExplored = 0;
int dfsPathLengthValue = 0;

/* =========================================================
   BFS PATH DATA
   ========================================================= */

Node bfsPathNodes[ROWS * COLS];

int bfsPathLength = 0;
int bfsPathStep = 0;
int bfsPathAnimating = 0;

/* =========================================================
   DFS PATH DATA
   ========================================================= */

Node dfsPathNodes[ROWS * COLS];

int dfsPathLength = 0;
int dfsPathStep = 0;
int dfsPathAnimating = 0;

/* =========================================================
   ANIMATION TIMER
   ========================================================= */

float animationTimer = 0.0f;

/* =========================================================
   ROBOT DATA
   ========================================================= */

int robotRow = 0;
int robotCol = 0;

int robotMoving = 0;
int robotReachedGoal = 0;

int robotPathStep = 0;

float robotMoveTimer = 0.0f;

/* =========================================================
   CHECK VALID CELL
   ========================================================= */

int isValid(
    int row,
    int col,
    int visited[ROWS][COLS])
{
    if (row < 0 || row >= ROWS ||
        col < 0 || col >= COLS)
    {
        return 0;
    }

    if (grid[row][col] == 1)
    {
        return 0;
    }

    if (visited[row][col] == 1)
    {
        return 0;
    }

    return 1;
}

/* =========================================================
   RESET ROBOT
   ========================================================= */

void resetRobot()
{
    robotRow = startRow;
    robotCol = startCol;

    robotMoving = 0;
    robotReachedGoal = 0;

    robotPathStep = 0;
    robotMoveTimer = 0.0f;
}

/* =========================================================
   CLEAR BFS
   ========================================================= */

void clearBFS()
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            bfsVisited[row][col] = 0;
            bfsPath[row][col] = 0;
        }
    }

    bfsFront = 0;
    bfsRear = 0;

    bfsAnimating = 0;
    bfsFinished = 0;

    bfsNodesExplored = 0;
    bfsPathLengthValue = 0;

    bfsPathLength = 0;
    bfsPathStep = 0;
    bfsPathAnimating = 0;
}

/* =========================================================
   CLEAR DFS
   ========================================================= */

void clearDFS()
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            dfsVisited[row][col] = 0;
            dfsPath[row][col] = 0;
        }
    }

    dfsTop = 0;

    dfsAnimating = 0;
    dfsFinished = 0;

    dfsNodesExplored = 0;
    dfsPathLengthValue = 0;

    dfsPathLength = 0;
    dfsPathStep = 0;
    dfsPathAnimating = 0;
}

/* =========================================================
   PREPARE BFS PATH
   ========================================================= */

void prepareBFSPath()
{
    int currentRow = goalRow;
    int currentCol = goalCol;

    bfsPathLength = 0;

    while (!(currentRow == startRow &&
             currentCol == startCol))
    {
        bfsPathNodes[bfsPathLength++] =
            (Node){currentRow, currentCol};

        Node previous =
            bfsParent[currentRow][currentCol];

        currentRow = previous.row;
        currentCol = previous.col;
    }

    bfsPathNodes[bfsPathLength++] =
        (Node){startRow, startCol};

    bfsPathStep = 0;

    bfsPathAnimating = 1;

    bfsPathLengthValue =
        bfsPathLength - 1;
}

/* =========================================================
   PREPARE DFS PATH
   ========================================================= */

void prepareDFSPath()
{
    int currentRow = goalRow;
    int currentCol = goalCol;

    dfsPathLength = 0;

    while (!(currentRow == startRow &&
             currentCol == startCol))
    {
        dfsPathNodes[dfsPathLength++] =
            (Node){currentRow, currentCol};

        Node previous =
            dfsParent[currentRow][currentCol];

        currentRow = previous.row;
        currentCol = previous.col;
    }

    dfsPathNodes[dfsPathLength++] =
        (Node){startRow, startCol};

    dfsPathStep = 0;

    dfsPathAnimating = 1;

    dfsPathLengthValue =
        dfsPathLength - 1;
}

/* =========================================================
   START BFS
   ========================================================= */

void startBFS()
{
    clearBFS();
    resetRobot();

    bfsQueue[bfsRear++] =
        (Node){startRow, startCol};

    bfsVisited[startRow][startCol] = 1;

    bfsNodesExplored = 1;

    bfsAnimating = 1;
    bfsFinished = 0;

    animationTimer = 0.0f;

    TraceLog(
        LOG_INFO,
        "BFS Started!");
}

/* =========================================================
   BFS STEP
   ========================================================= */

void bfsStep()
{
    if (bfsFront >= bfsRear)
    {
        bfsAnimating = 0;
        bfsFinished = 1;

        TraceLog(
            LOG_INFO,
            "BFS: No path found!");

        return;
    }

    Node current =
        bfsQueue[bfsFront++];

    if (current.row == goalRow &&
        current.col == goalCol)
    {
        bfsAnimating = 0;

        prepareBFSPath();

        TraceLog(
            LOG_INFO,
            "BFS: Exploration complete!");

        return;
    }

    int directions[4][2] =
        {
            {-1, 0},
            {1, 0},
            {0, -1},
            {0, 1}};

    for (int i = 0; i < 4; i++)
    {
        int newRow =
            current.row + directions[i][0];

        int newCol =
            current.col + directions[i][1];

        if (isValid(
                newRow,
                newCol,
                bfsVisited))
        {
            bfsVisited[newRow][newCol] = 1;

            bfsNodesExplored++;

            bfsParent[newRow][newCol] =
                current;

            bfsQueue[bfsRear++] =
                (Node){newRow, newCol};
        }
    }
}

/* =========================================================
   BFS PATH ANIMATION
   ========================================================= */

void bfsPathStepAnimation()
{
    if (bfsPathStep >= bfsPathLength)
    {
        bfsPathAnimating = 0;
        bfsFinished = 1;

        resetRobot();

        robotMoving = 1;

        TraceLog(
            LOG_INFO,
            "BFS: Shortest path displayed!");

        return;
    }

    Node current =
        bfsPathNodes[bfsPathLength - 1 - bfsPathStep];

    bfsPath[current.row][current.col] = 1;

    bfsPathStep++;
}

/* =========================================================
   START DFS
   ========================================================= */

void startDFS()
{
    clearDFS();
    resetRobot();

    dfsStack[dfsTop++] =
        (Node){startRow, startCol};

    dfsVisited[startRow][startCol] = 1;

    dfsNodesExplored = 1;

    dfsAnimating = 1;
    dfsFinished = 0;

    animationTimer = 0.0f;

    TraceLog(
        LOG_INFO,
        "DFS Started!");
}

/* =========================================================
   DFS STEP
   ========================================================= */

void dfsStep()
{
    if (dfsTop <= 0)
    {
        dfsAnimating = 0;
        dfsFinished = 1;

        TraceLog(
            LOG_INFO,
            "DFS: No path found!");

        return;
    }

    Node current =
        dfsStack[--dfsTop];

    if (current.row == goalRow &&
        current.col == goalCol)
    {
        dfsAnimating = 0;

        prepareDFSPath();

        TraceLog(
            LOG_INFO,
            "DFS: Exploration complete!");

        return;
    }

    int directions[4][2] =
        {
            {-1, 0},
            {1, 0},
            {0, -1},
            {0, 1}};

    for (int i = 0; i < 4; i++)
    {
        int newRow =
            current.row + directions[i][0];

        int newCol =
            current.col + directions[i][1];

        if (isValid(
                newRow,
                newCol,
                dfsVisited))
        {
            dfsVisited[newRow][newCol] = 1;

            dfsNodesExplored++;

            dfsParent[newRow][newCol] =
                current;

            dfsStack[dfsTop++] =
                (Node){newRow, newCol};
        }
    }
}

/* =========================================================
   DFS PATH ANIMATION
   ========================================================= */

void dfsPathStepAnimation()
{
    if (dfsPathStep >= dfsPathLength)
    {
        dfsPathAnimating = 0;
        dfsFinished = 1;

        resetRobot();

        robotMoving = 1;

        TraceLog(
            LOG_INFO,
            "DFS: Path displayed!");

        return;
    }

    Node current =
        dfsPathNodes[dfsPathLength - 1 - dfsPathStep];

    dfsPath[current.row][current.col] = 1;

    dfsPathStep++;
}

/* =========================================================
   ROBOT MOVEMENT - BFS
   ========================================================= */

void moveRobotAlongBFSPath()
{
    if (!robotMoving)
        return;

    if (bfsPathLength <= 0)
        return;

    if (robotPathStep >= bfsPathLength)
    {
        robotMoving = 0;
        robotReachedGoal = 1;

        TraceLog(
            LOG_INFO,
            "Robot reached goal using BFS!");

        return;
    }

    Node current =
        bfsPathNodes[bfsPathLength - 1 - robotPathStep];

    robotRow = current.row;
    robotCol = current.col;

    robotPathStep++;
}

/* =========================================================
   ROBOT MOVEMENT - DFS
   ========================================================= */

void moveRobotAlongDFSPath()
{
    if (!robotMoving)
        return;

    if (dfsPathLength <= 0)
        return;

    if (robotPathStep >= dfsPathLength)
    {
        robotMoving = 0;
        robotReachedGoal = 1;

        TraceLog(
            LOG_INFO,
            "Robot reached goal using DFS!");

        return;
    }

    Node current =
        dfsPathNodes[dfsPathLength - 1 - robotPathStep];

    robotRow = current.row;
    robotCol = current.col;

    robotPathStep++;
}

/* =========================================================
   RESET RESULTS
   ========================================================= */

void resetResults()
{
    clearBFS();
    clearDFS();

    resetRobot();

    TraceLog(
        LOG_INFO,
        "Results reset!");
}

/* =========================================================
   DRAW ROBOT
   ========================================================= */

void drawRobot(
    int row,
    int col)
{
    int x = col * CELL_SIZE;
    int y = row * CELL_SIZE;

    // Body
    DrawRectangle(
        x + 11,
        y + 20,
        28,
        23,
        YELLOW);

    // Head
    DrawRectangle(
        x + 8,
        y + 7,
        34,
        20,
        YELLOW);

    // Antenna
    DrawLine(
        x + 25,
        y + 7,
        x + 25,
        y + 1,
        DARKGRAY);

    DrawCircle(
        x + 25,
        y + 1,
        3,
        RED);

    // Eyes
    DrawCircle(
        x + 18,
        y + 16,
        3,
        BLACK);

    DrawCircle(
        x + 32,
        y + 16,
        3,
        BLACK);

    // Mouth
    DrawLine(
        x + 18,
        y + 23,
        x + 32,
        y + 23,
        DARKGRAY);

    // Left arm
    DrawLine(
        x + 11,
        y + 25,
        x + 4,
        y + 33,
        DARKGRAY);

    // Right arm
    DrawLine(
        x + 39,
        y + 25,
        x + 46,
        y + 33,
        DARKGRAY);

    // Left leg
    DrawLine(
        x + 19,
        y + 43,
        x + 16,
        y + 48,
        DARKGRAY);

    // Right leg
    DrawLine(
        x + 31,
        y + 43,
        x + 34,
        y + 48,
        DARKGRAY);
}

/* =========================================================
   DRAW LEGEND ITEM
   ========================================================= */

void drawLegendItem(
    Font font,
    int x,
    int y,
    Color color,
    const char *text)
{
    DrawRectangle(
        x,
        y + 3,
        18,
        18,
        color);

    DrawRectangleLines(
        x,
        y + 3,
        18,
        18,
        GRAY);

    DrawTextEx(
        font,
        text,
        (Vector2){x + 28, y},
        16,
        0.5f,
        DARKGRAY);
}

/* =========================================================
   STATISTICS PANEL
   ========================================================= */

void drawStatisticsPanel(
    Font arialFont,
    int panelX,
    int panelHeight)
{
    DrawRectangle(
        panelX,
        0,
        PANEL_WIDTH,
        panelHeight,
        LIGHTGRAY);

    /* =====================================================
       TITLE
       ===================================================== */

    DrawTextEx(
        arialFont,
        "SMART ROBOT",
        (Vector2){panelX + 30, 18},
        24,
        0.5f,
        BLACK);

    DrawTextEx(
        arialFont,
        "PATH PLANNER",
        (Vector2){panelX + 25, 47},
        24,
        0.5f,
        BLACK);

    DrawLine(
        panelX + 20,
        82,
        panelX + PANEL_WIDTH - 20,
        82,
        WHITE);

    /* =====================================================
       BFS
       ===================================================== */

    DrawTextEx(
        arialFont,
        "BFS",
        (Vector2){panelX + 25, 96},
        22,
        0.5f,
        BLUE);

    DrawTextEx(
        arialFont,
        "Queue",
        (Vector2){panelX + 90, 99},
        16,
        0.5f,
        DARKGRAY);

    DrawTextEx(
        arialFont,
        TextFormat(
            "Nodes Explored: %d",
            bfsNodesExplored),
        (Vector2){panelX + 25, 128},
        16,
        0.5f,
        DARKGRAY);

    DrawTextEx(
        arialFont,
        TextFormat(
            "Path Length: %d",
            bfsPathLengthValue),
        (Vector2){panelX + 25, 153},
        16,
        0.5f,
        DARKGRAY);

    if (bfsAnimating)
    {
        DrawTextEx(
            arialFont,
            "Exploring...",
            (Vector2){panelX + 25, 178},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (bfsPathAnimating)
    {
        DrawTextEx(
            arialFont,
            "Building Path...",
            (Vector2){panelX + 25, 178},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (robotMoving &&
             bfsPathLength > 0)
    {
        DrawTextEx(
            arialFont,
            "Robot Moving...",
            (Vector2){panelX + 25, 178},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (robotReachedGoal &&
             bfsPathLength > 0)
    {
        DrawTextEx(
            arialFont,
            "Goal Reached!",
            (Vector2){panelX + 25, 178},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (bfsFinished)
    {
        DrawTextEx(
            arialFont,
            "Completed",
            (Vector2){panelX + 25, 178},
            16,
            0.5f,
            DARKGRAY);
    }
    else
    {
        DrawTextEx(
            arialFont,
            "Ready",
            (Vector2){panelX + 25, 178},
            16,
            0.5f,
            DARKGRAY);
    }

    DrawLine(
        panelX + 20,
        207,
        panelX + PANEL_WIDTH - 20,
        207,
        GRAY);

    /* =====================================================
       DFS
       ===================================================== */

    DrawTextEx(
        arialFont,
        "DFS",
        (Vector2){panelX + 25, 220},
        22,
        0.5f,
        PURPLE);

    DrawTextEx(
        arialFont,
        "Stack",
        (Vector2){panelX + 90, 223},
        16,
        0.5f,
        DARKGRAY);

    DrawTextEx(
        arialFont,
        TextFormat(
            "Nodes Explored: %d",
            dfsNodesExplored),
        (Vector2){panelX + 25, 252},
        16,
        0.5f,
        DARKGRAY);

    DrawTextEx(
        arialFont,
        TextFormat(
            "Path Length: %d",
            dfsPathLengthValue),
        (Vector2){panelX + 25, 277},
        16,
        0.5f,
        DARKGRAY);

    if (dfsAnimating)
    {
        DrawTextEx(
            arialFont,
            "Exploring...",
            (Vector2){panelX + 25, 302},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (dfsPathAnimating)
    {
        DrawTextEx(
            arialFont,
            "Building Path...",
            (Vector2){panelX + 25, 302},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (robotMoving &&
             dfsPathLength > 0)
    {
        DrawTextEx(
            arialFont,
            "Robot Moving...",
            (Vector2){panelX + 25, 302},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (robotReachedGoal &&
             dfsPathLength > 0)
    {
        DrawTextEx(
            arialFont,
            "Goal Reached!",
            (Vector2){panelX + 25, 302},
            16,
            0.5f,
            DARKGRAY);
    }
    else if (dfsFinished)
    {
        DrawTextEx(
            arialFont,
            "Completed",
            (Vector2){panelX + 25, 302},
            16,
            0.5f,
            DARKGRAY);
    }
    else
    {
        DrawTextEx(
            arialFont,
            "Ready",
            (Vector2){panelX + 25, 302},
            16,
            0.5f,
            DARKGRAY);
    }

    DrawLine(
        panelX + 20,
        331,
        panelX + PANEL_WIDTH - 20,
        331,
        GRAY);

    /* =====================================================
       LEGEND
       ===================================================== */

    DrawTextEx(
        arialFont,
        "LEGEND",
        (Vector2){panelX + 25, 343},
        20,
        0.5f,
        DARKGRAY);

    // BFS Explored
    drawLegendItem(
        arialFont,
        panelX + 25,
        370,
        YELLOW,
        "BFS Explored");

    // DFS Explored
    drawLegendItem(
        arialFont,
        panelX + 155,
        370,
        PURPLE,
        "DFS Explored");

    // BFS Path
    drawLegendItem(
        arialFont,
        panelX + 25,
        400,
        BLUE,
        "BFS Path");

    // DFS Path
    drawLegendItem(
        arialFont,
        panelX + 155,
        400,
        ORANGE,
        "DFS Path");

    // Obstacle
    drawLegendItem(
        arialFont,
        panelX + 25,
        430,
        DARKGRAY,
        "Obstacle");

    // Robot
    drawLegendItem(
        arialFont,
        panelX + 155,
        430,
        YELLOW,
        "Robot");

    // Start
    drawLegendItem(
        arialFont,
        panelX + 25,
        460,
        GREEN,
        "Start");

    // Goal
    drawLegendItem(
        arialFont,
        panelX + 155,
        460,
        RED,
        "Goal");
}

/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    int gridWidth =
        COLS * CELL_SIZE;

    int screenHeight =
        ROWS * CELL_SIZE;

    int screenWidth =
        gridWidth + PANEL_WIDTH;

    /* =====================================================
       START + GOAL
       ===================================================== */

    grid[startRow][startCol] = 2;
    grid[goalRow][goalCol] = 3;

    /* =====================================================
       WINDOW
       ===================================================== */

    InitWindow(
        screenWidth,
        screenHeight,
        "Smart Robot Path Planner");

    SetTargetFPS(60);

    /* =====================================================
       ARIAL FONT
       ===================================================== */

    Font arialFont = LoadFont(
        "/System/Library/Fonts/Supplemental/Arial.ttf");

    /* =====================================================
       ROBOT INITIALIZATION
       ===================================================== */

    resetRobot();

    /* =====================================================
       MAIN LOOP
       ===================================================== */

    while (!WindowShouldClose())
    {

        /* =================================================
           MOUSE - OBSTACLE
           ================================================= */

        if (!bfsAnimating &&
            !dfsAnimating &&
            !bfsPathAnimating &&
            !dfsPathAnimating &&
            !robotMoving)
        {
            if (IsMouseButtonPressed(
                    MOUSE_BUTTON_LEFT))
            {
                Vector2 mousePosition =
                    GetMousePosition();

                int col =
                    mousePosition.x / CELL_SIZE;

                int row =
                    mousePosition.y / CELL_SIZE;

                if (row >= 0 &&
                    row < ROWS &&
                    col >= 0 &&
                    col < COLS)
                {
                    if (grid[row][col] == 0)
                    {
                        grid[row][col] = 1;
                    }
                }
            }
        }

        /* =================================================
           B = BFS
           ================================================= */

        if (IsKeyPressed(KEY_B))
        {
            if (!bfsAnimating &&
                !dfsAnimating &&
                !bfsPathAnimating &&
                !dfsPathAnimating &&
                !robotMoving)
            {
                clearDFS();

                startBFS();
            }
        }

        /* =================================================
           D = DFS
           ================================================= */

        if (IsKeyPressed(KEY_D))
        {
            if (!bfsAnimating &&
                !dfsAnimating &&
                !bfsPathAnimating &&
                !dfsPathAnimating &&
                !robotMoving)
            {
                clearBFS();

                startDFS();
            }
        }

        /* =================================================
           R = RESET RESULTS
           ================================================= */

        if (IsKeyPressed(KEY_R))
        {
            if (!bfsAnimating &&
                !dfsAnimating &&
                !bfsPathAnimating &&
                !dfsPathAnimating &&
                !robotMoving)
            {
                resetResults();
            }
        }

        /* =================================================
           EXPLORATION + PATH ANIMATION
           ================================================= */

        if (bfsAnimating ||
            dfsAnimating ||
            bfsPathAnimating ||
            dfsPathAnimating)
        {
            animationTimer +=
                GetFrameTime();

            if (animationTimer >=
                ANIMATION_DELAY)
            {
                animationTimer = 0.0f;

                if (bfsAnimating)
                {
                    bfsStep();
                }
                else if (bfsPathAnimating)
                {
                    bfsPathStepAnimation();
                }

                if (dfsAnimating)
                {
                    dfsStep();
                }
                else if (dfsPathAnimating)
                {
                    dfsPathStepAnimation();
                }
            }
        }

        /* =================================================
           ROBOT MOVEMENT
           ================================================= */

        if (robotMoving)
        {
            robotMoveTimer +=
                GetFrameTime();

            if (robotMoveTimer >=
                ROBOT_MOVE_DELAY)
            {
                robotMoveTimer = 0.0f;

                if (bfsPathLength > 0)
                {
                    moveRobotAlongBFSPath();
                }
                else if (dfsPathLength > 0)
                {
                    moveRobotAlongDFSPath();
                }
            }
        }

        /* =================================================
           DRAW
           ================================================= */

        BeginDrawing();

        ClearBackground(RAYWHITE);

        /* =================================================
           DRAW GRID
           ================================================= */

        for (int row = 0; row < ROWS; row++)
        {
            for (int col = 0; col < COLS; col++)
            {
                int x =
                    col * CELL_SIZE;

                int y =
                    row * CELL_SIZE;

                /* =========================
                   BFS EXPLORED
                   ========================= */

                if (bfsVisited[row][col])
                {
                    DrawRectangle(
                        x + 2,
                        y + 2,
                        CELL_SIZE - 4,
                        CELL_SIZE - 4,
                        YELLOW);
                }

                /* =========================
                   BFS PATH
                   ========================= */

                if (bfsPath[row][col])
                {
                    DrawRectangle(
                        x + 5,
                        y + 5,
                        CELL_SIZE - 10,
                        CELL_SIZE - 10,
                        BLUE);
                }

                /* =========================
                   DFS EXPLORED
                   ========================= */

                if (dfsVisited[row][col])
                {
                    DrawRectangle(
                        x + 2,
                        y + 2,
                        CELL_SIZE - 4,
                        CELL_SIZE - 4,
                        PURPLE);
                }

                /* =========================
                   DFS PATH
                   ========================= */

                if (dfsPath[row][col])
                {
                    DrawRectangle(
                        x + 5,
                        y + 5,
                        CELL_SIZE - 10,
                        CELL_SIZE - 10,
                        ORANGE);
                }

                /* =========================
                   OBSTACLE
                   ========================= */

                if (grid[row][col] == 1)
                {
                    DrawRectangle(
                        x + 2,
                        y + 2,
                        CELL_SIZE - 4,
                        CELL_SIZE - 4,
                        DARKGRAY);
                }

                /* =========================
                   START
                   ========================= */

                if (grid[row][col] == 2)
                {
                    DrawRectangle(
                        x + 2,
                        y + 2,
                        CELL_SIZE - 4,
                        CELL_SIZE - 4,
                        GREEN);

                    DrawTextEx(
                        arialFont,
                        "S",
                        (Vector2){x + 16, y + 5},
                        35,
                        0.5f,
                        WHITE);
                }

                /* =========================
                   GOAL
                   ========================= */

                if (grid[row][col] == 3)
                {
                    DrawRectangle(
                        x + 2,
                        y + 2,
                        CELL_SIZE - 4,
                        CELL_SIZE - 4,
                        RED);

                    DrawTextEx(
                        arialFont,
                        "G",
                        (Vector2){x + 16, y + 5},
                        35,
                        0.5f,
                        WHITE);
                }

                /* =========================
                   GRID LINE
                   ========================= */

                DrawRectangleLines(
                    x,
                    y,
                    CELL_SIZE,
                    CELL_SIZE,
                    LIGHTGRAY);
            }
        }

        /* =================================================
           DRAW ROBOT
           ================================================= */

        if (robotMoving ||
            robotReachedGoal)
        {
            drawRobot(
                robotRow,
                robotCol);
        }

        /* =================================================
           STATISTICS + LEGEND PANEL
           ================================================= */

        drawStatisticsPanel(
            arialFont,
            gridWidth,
            screenHeight);

        EndDrawing();
    }

    /* =====================================================
       CLEANUP
       ===================================================== */

    UnloadFont(arialFont);

    CloseWindow();

    return 0;
}