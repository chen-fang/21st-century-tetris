
/*
 * Author: Chafic Najjar
 * Tetris game written in C++ using SDL 2.0
 */

#include "tetris.hpp"
#include "tetromino.hpp"
#include "board.hpp"
#include "utilities.hpp"

#include <random>

std::random_device rd;
std::mt19937 gen(rd());

/* Screen resolution */
const int Tetris::SCREEN_WIDTH  = 500;
const int Tetris::SCREEN_HEIGHT = 640;

const float Tetris::colors[NCOLORS][4] = {
    { 255, 0  , 0  , 255 },      // red
    { 0  , 255, 0  , 255 },      // blue
    { 0  , 0  , 255, 255 },      // turquoise
    { 255, 255, 0  , 255 },      // purple
    { 0  , 255, 255, 255 },      // brown
    { 255, 0  , 255, 255 }       // yellow
};


Tetris::Tetris(int argc, char *argv[]) {

    /* Initialize audio, CD-ROM, event handling, file I/O, joystick handling, threading, timers and videos */
    SDL_Init(SDL_INIT_EVERYTHING);

    /* Window and renderer */
    window = SDL_CreateWindow( "Tetris Unleashed!",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, 
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN );

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

    /* Game objects */
    board        = new Board();
    tetro        = new Tetromino( rand()%7, rand()%NCOLORS );       // current tetromino
    next_tetro   = new Tetromino( rand()%7, rand()%NCOLORS );       // next tetromino

    /* Sounds */

    /* Controllers */

    /* Fonts */
    TTF_Init();
    font_color = { 255, 255, 255 };
    font_image_tetris = renderText("Tetris Unleashed!", "resources/fonts/bitwise.ttf", font_color, 16, renderer);
    font_image_score_text = renderText("Score: ", "resources/fonts/bitwise.ttf", font_color, 20, renderer);
    font_image_score = renderText(std::to_string(score), "resources/fonts/bitwise.ttf", font_color, 20, renderer);
    font_image_new_game = renderText("New game", "resources/fonts/bitwise.ttf", font_color, 20, renderer);
    font_image_quit = renderText("Quit", "resources/fonts/bitwise.ttf", font_color, 20, renderer);
    font_image_game_over = renderText("Game over!", "resources/fonts/bitwise.ttf", font_color, 16, renderer);

    /* Scores */

    xoffset = 0;

    rotate_left    = false;
    shifted        = false;
    zoom_down       = false;
    speedup        = false;

    launch_tetro    = false;

    newgamedown    = false;
    newgameup      = false;
    quitdown       = false;
    quitup         = false;

    gameover       = false;
    done           = false;

    delete_row     = false;
    bonus           = 3; 
    bonus_counter   = 0;
    score           = 0;
    render_score   = true;

    gameoffset    = 20.0f;

    acceleration  = 0.005f;

    this_time      = 0.0f;
    last_time      = 0.0f;
    delta_time     = 0.0f;
    time_till_drop= 0.3f;
    time_counter  = 0.0f;

    // Coordinates of the "New Game" button
    // Also used for "Quit" button
    newgamex1 = gameoffset+board->WIDTH+board->BLOCK_WIDTH;       // left
    newgamex2 = gameoffset+board->WIDTH+8*board->BLOCK_WIDTH;     // right
    newgamey1 = board->HEIGHT-4*board->BLOCK_HEIGHT;              // down
    newgamey2 = board->HEIGHT-6*board->BLOCK_HEIGHT;              // up


    // At the start of the game:
    // x position of (0, 0) block of tetro is int(15/2) = 7 which is the exact horizontal middle of board
    // y position of (0, 0) block of tetro is 0 which is the top of the board
    tetro->set_position(int(board->NUMCOLS/2), 0);

    // Position next_tetro at the upper right of the window, outside of the board
    next_tetro->set_position(board->NUMCOLS+5,int(0.3*board->NUMROWS)); 

}
 
// Get player input
void Tetris::input() {

    // Queuing events
    SDL_Event event;
    while (SDL_PollEvent(&event)) { 

        // Clicking 'x' or pressing F4
        if (event.type == SDL_QUIT) 
            done = true;

        // Key is pressed
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: 
                    done = true; 
                    break;
                case SDLK_a: case SDLK_LEFT:
                    xoffset = -1;
                    shifted = true;
                    break;
                case SDLK_d: case SDLK_RIGHT:
                    xoffset = 1;
                    shifted = true;
                    break;
                case SDLK_w: case SDLK_UP: 
                    if(tetro->type != 4)        // type 4 is the square
                        rotate_left = true;
                    break;
                case SDLK_s: case SDLK_DOWN:
                    speedup = true;
                    break;
                case SDLK_SPACE:
                    zoom_down = true; 
                    break;
                default: 
                    break;
            }

        }

        // Key is released
        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_s: case SDLK_DOWN:
                    speedup = false;
                    break;
                default:
                    break;
            } 
        }

        // Mouse moves
        if (event.type == SDL_MOUSEMOTION) {

            // Outside of the board
            if (event.motion.x > board->WIDTH + gameoffset) 
                SDL_ShowCursor(1);          // show cursor
            
            // Inside the board
            else
                SDL_ShowCursor(0);          // don't show cursor
        }

        // Mouse button clicked
        if (event.type == SDL_MOUSEBUTTONDOWN)
            switch (event.button.button) {

                // Left mouse button clicked
                case SDL_BUTTON_LEFT:
                    if (event.button.x > newgamex1 && event.button.x < newgamex2) {

                        // And mouse cursor is on "New Game" button 
                        if (event.button.y > newgamey2 && event.button.y < newgamey1)
                            newgamedown = true;

                        // And mouse cursor is on "Quit" button
                        else if (event.button.y > newgamey2+4*board->BLOCK_HEIGHT && event.button.y < newgamey1+4*board->BLOCK_HEIGHT)
                            quitdown = true;
                    }
                    break;
                default:
                    break;
            }

        // Mouse button released 
        if (event.type == SDL_MOUSEBUTTONUP)
            switch (event.button.button) {

                // Left mouse button released
                case SDL_BUTTON_LEFT:
                    if (event.button.x > newgamex1 && event.button.x < newgamex2) {

                        // And mouse cursor is on "New Game" button
                        if (event.button.y > newgamey2 && event.button.y < newgamey1)
                            newgameup = true;

                        // And mouse cursor is on "Quit" button
                        else if (event.button.y > newgamey2+4*board->BLOCK_HEIGHT && event.button.y < newgamey1+4*board->BLOCK_HEIGHT)
                            quitup = true;
                    }
                    break;
                default:
                    break;
            }
    }
}

// Update game values
void Tetris::update() { 

    frame_rate(&last_time, &this_time, &delta_time);

    // No gameover (tetromino hasn't crossed the top border)
    if (!gameover) { 

        // Tetromino has landed
        if (launch_tetro) { 
            int x, y;

            // Check if tetromino has trespassed over the top border of the board
            for (int i = 0; i < tetro->SIZE; i++) { 

                x = tetro->get_block_x(i);
                y = tetro->get_block_y(i);

                // If any block touches the top border of the board, then it's game over
                if (y <= 0) { 
                    gameover = true;
                    return; 
                }

                board->color[y][x] = tetro->color; // update color in corresponding block position

            }

            // New tetromino
            Tetromino *new_tetro = new Tetromino(rand()%7, rand()%NCOLORS); // assign random type and color to new tetromino
            new_tetro->set_position(next_tetro->X, next_tetro->Y);          // new tetrominoe is positioned under "Next Piece"

            delete [] tetro;                        // delete allocated memory
            tetro = next_tetro;                     // update falling tetromino
            tetro->set_position(int(board->NUMCOLS/2), 0);  // position the newly falling tetromino
            next_tetro = new_tetro;                 // update next_tetro to point to tetromino under "Next Piece"
            launch_tetro = false;                       // old falling tetromino has landed and new tetromino has already been generated
            zoom_down = false;                       // previous tetromino has landed so zoom_down is necessarly false
        }

        // No zoom_down
        if (!zoom_down) {

            // Player has rotated tetromino
            if (rotate_left)
                tetro->rotate_left();

            // Update tetromino position on the x-axis
            tetro->X += xoffset; 

            time_counter += delta_time;

            // time_till_drop = 0.3;
            // delta_time ~ 0.017 seconds
            // Tetromino falls one block for every 0.3/0.017 = 17 game loops
            if (time_counter >= time_till_drop) { 
                tetro->Y++; 
                time_counter = 0.0f;
            }

            // No speedup
            if (!speedup) 
                // Drop speed proportional to score
                time_till_drop = 0.3f - score*acceleration; 

            // Speedup ('s' or 'down') was pressed
            else
                time_till_drop = 0.02f; // 2x slower than zoom_down
        }

        // zoom_down
        else 
            // The speed of fall is not decreased, the tetromino falls at speed proportional to the game loop
            tetro->Y++; 

        // Check if tetromino is in an acceptable place. If not, undo previous move(s)
        int x, y;
        
        // Collision detection
        for (int i = 0; i < tetro->SIZE; i++) {

            // Coordinates of falling tetromino
            x = tetro->get_block_x(i);
            y = tetro->get_block_y(i); 

            // Block crosses wall after rotation or translation 
            if (x < 0 || x >= board->NUMCOLS) {

                // If it exceeds the wall because of rotation
                if (rotate_left) 
                    tetro->rotate_right();       // neutralize the left rotation 

                // Else it exceeds the wall because of translation
                else 
                    tetro->X -= xoffset;        // neutralize shift
            }

            // Block bypasses top border of the board 
            else if (y >= board->NUMROWS) {
                launch_tetro = true;                // cast a new tetrimino
                tetro->set_block_y(i, board->NUMROWS-1); // change the value of Y so that block(s) of the (old) tetromino is/are above the blue line
            }

            // Block is on the board
            else if (y >= 0) 

                // Block touched another block
                if (board->color[y][x] != -1) { 

                    // Tetromino rotates and collides with a block
                    if (rotate_left) { 
                        tetro->rotate_right();           // neutralize
                        rotate_left = false;                // done rotating
                    } 
                    
                    // Tetromino is shifted into another block
                    else if (shifted) { 
                        tetro->X -= xoffset;            // neutralize
                        shifted = false;                // done shifting
                    } 
                    
                    // Block is neither rotated or shifted
                    else { 
                        tetro->Y--;                     // neutralize: tetromino goes up
                        launch_tetro = true;                // done landing
                    }

                    // Next block to test
                    x = tetro->get_block_x(i);
                    y = tetro->get_block_y(i);
                }
        }

        // If row is full, shift board down 
        for (int i = board->NUMROWS-1; i >= 0; i--) {          // test every row

            if (!delete_row) {
                delete_row = true;                      // true until proven false
                for (int j = 0; j < board->NUMCOLS; j++) {     // check each block of the row 
                    if (board->color[i][j] == -1) {            // if there exists at least one empty block 
                        delete_row = false;             // do not delete any row
                        break; 
                    }
                }
            }

            // If delete_row is true, shift board down
            else 
                for (int j = 0; j < board->NUMCOLS; j++)
                    board->color[i+1][j] = board->color[i][j];

        }

        // Row was deleted
        if (delete_row) {
            score++;                    // increment score
            bonus_counter++;            // increment bonus counter 
            render_score = true;
        }

        else 
            bonus_counter = 0;          // tetromino has landed but no row was deleted
        
        delete_row = false;             // row now deleted

    }

    // Done rotating and shifting
    rotate_left = false; 
    shifted = false;
    xoffset = 0;

    // 4 rows deleted in a row (no pun intended)
    if (bonus_counter == 4) { 
           score += bonus;      // you get 3 points
           bonus_counter = 0;
    }

}

// Create "New Game" and "Quit" buttons
void Tetris::create_button(int x, int y, int width, int height, int k) {

        SDL_Rect rect = { x, y, width, height };
        SDL_SetRenderDrawColor(renderer, colors[k][0], colors[k][1], colors[k][2], colors[k][3]);
        SDL_RenderFillRect(renderer, &rect);

}

// Render Tetromino block
void Tetris::draw_block(int x, int y, int k) {

    SDL_Rect block = {x, y, int(board->BLOCK_WIDTH), int(board->BLOCK_HEIGHT)};
    SDL_SetRenderDrawColor(renderer, colors[k][0], colors[k][1], colors[k][2], colors[k][3]);
    SDL_RenderFillRect(renderer, &block);

}

// Render result
void Tetris::render() {

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
    SDL_RenderClear(renderer);


    int x = (next_tetro->X-3)*board->BLOCK_WIDTH; 
    int y = gameoffset; 

    renderTexture(font_image_tetris, renderer, x, y);

    // Render score text
    renderTexture(font_image_score_text, renderer, x, y + board->BLOCK_WIDTH);

    // Render score
    if (render_score) {
        font_image_score = renderText(std::to_string(score), "resources/fonts/bitwise.ttf", font_color, 20, renderer);
        render_score = false;
    }
    renderTexture(font_image_score, renderer, x + 60, y + board->BLOCK_WIDTH);
    
    int tetro_x, tetro_y;

    // Draw tetromino squares
    for (int i = 0; i < tetro->SIZE; i++) {

        // Get new coordinates
        tetro_x = float(tetro->get_block_x(i))*board->BLOCK_WIDTH + gameoffset;
        tetro_y = float(tetro->get_block_y(i))*board->BLOCK_HEIGHT + gameoffset;

        draw_block(tetro_x, tetro_y, tetro->color);

    }

    if (!gameover)

        // Draw next tetromino
        for (int i = 0; i < next_tetro->SIZE; i++) {


            // Get new coordinates
            tetro_x = float(next_tetro->get_block_x(i))*board->BLOCK_WIDTH;
            tetro_y = float(next_tetro->get_block_y(i))*board->BLOCK_HEIGHT;

            draw_block(tetro_x, tetro_y, next_tetro->color);

        }

    // This is the board. Non-active tetrominos live here.
    for (int i = 0; i < board->NUMROWS; i++)
        for (int j = 0; j < board->NUMCOLS; j++)

            if (board->color[i][j]!=-1) {


                // Get new coordinates
                tetro_x = float(j)*board->BLOCK_WIDTH + gameoffset;
                tetro_y = float(i)*board->BLOCK_HEIGHT + gameoffset;

                draw_block(tetro_x, tetro_y, board->color[i][j]);

            }

    
    // Box surrounding board

    // Set color to blue
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

    // Draw left border
    SDL_RenderDrawLine(renderer, gameoffset, gameoffset, gameoffset, gameoffset+board->HEIGHT);

    // Draw right border
    SDL_RenderDrawLine(renderer, gameoffset+board->WIDTH, gameoffset, gameoffset+board->WIDTH, gameoffset+board->HEIGHT);

    // Draw upper border
    SDL_RenderDrawLine(renderer, gameoffset, gameoffset, gameoffset+board->WIDTH, gameoffset);

    // Draw bottom border
    SDL_RenderDrawLine(renderer, gameoffset, gameoffset+board->HEIGHT, gameoffset+board->WIDTH, gameoffset+board->HEIGHT);

    // If game is over, display "Game Over!"
    if (gameover)
        renderTexture(font_image_game_over, renderer, newgamex1, SCREEN_HEIGHT-newgamey1+4*board->BLOCK_WIDTH);

    // Create "New Game" button
    create_button(newgamex1, newgamey2, 7*board->BLOCK_WIDTH, 2*board->BLOCK_HEIGHT, 2);

    // Render "New Game" font
    renderTexture(font_image_new_game, renderer, newgamex1+10, newgamey2+10);

    // Create "Quit" button
    create_button(newgamex1, newgamey2+4*board->BLOCK_HEIGHT, 7*board->BLOCK_WIDTH, 2*board->BLOCK_HEIGHT, 0);

    // Render "Quit" font
    renderTexture(font_image_quit, renderer, newgamex1+10, newgamey2+4*board->BLOCK_HEIGHT+10);

    // Swap buffers
    SDL_RenderPresent(renderer);

}

// Restarts game
void Tetris::reset() { 

    for(int i = 0; i < board->NUMROWS; i++)
        for(int j = 0; j < board->NUMCOLS; j++)
            board->color[i][j] = -1; 

    score = 0;
    render_score = true;

    // release allocated memory
    delete [] tetro;
    delete [] next_tetro;

    tetro = new Tetromino(rand()%7, rand()%NCOLORS ); 
    next_tetro = new Tetromino(rand()%7, rand()%NCOLORS );
    tetro->set_position(int(board->NUMCOLS/2),0);
    next_tetro->set_position(board->NUMCOLS+5,int(0.3*board->NUMROWS));

    rotate_left         = false;
    shifted         = false;
    zoom_down        = false;
    
    launch_tetro        = false;
    gameover        = false;
    newgameup       = false;
    newgamedown     = false;

    bonus_counter   = 0;

}

void Tetris::clean_up() {
    SDL_DestroyTexture(font_image_tetris);
    SDL_DestroyTexture(font_image_score_text);
    SDL_DestroyTexture(font_image_score);
    SDL_DestroyTexture(font_image_new_game);
    SDL_DestroyTexture(font_image_quit);
    SDL_DestroyTexture(font_image_game_over);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Execute game loop
void Tetris::execute() {

    do {

        input();
        update();
        render();

        if(newgameup && newgamedown)
            reset();             

        if(quitup && quitdown)
            done = true;

    } while(!done);

    clean_up();
}

void Tetris::frame_rate(float *last_time, float *this_time, float *delta_time) {

    // Get number of milliseconds since SDL_Init() of the previous frame
    *last_time = *this_time; 

    // Get number of milliseconds since SDL_Init()
    *this_time = SDL_GetTicks(); 

    // Variation of time between each game loop
    *delta_time = (*this_time - *last_time) / 1000.0;         // dividing by 1000 to convert in seconds

}

