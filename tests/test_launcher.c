#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include "game_launcher.h"

static void test_game_process_init_and_cleanup(void) {
    printf("[TEST] Running test_game_process_init_and_cleanup...\n");
    GameProcess game;
    game_process_init(&game);
    assert(game.pid == -1);
    assert(game.pipe_read_fd == -1);
    assert(!game.is_running);
    game_process_cleanup(&game);
    printf("  -> PASS\n");
}

static void test_launch_game_and_wait(void) {
    printf("[TEST] Running test_launch_game_and_wait...\n");
    GameProcess game;
    game_process_init(&game);

    char *const argv[] = { "./game", "--ticks", "2", "--interval", "10", NULL };
    int rc = launch_game(&game, "./game", argv);
    assert(rc == 0);
    assert(game.pid > 0);
    assert(game.pipe_read_fd >= 0);
    assert(game.is_running);

    /* Read events and wait for game to finish */
    char buffer[256];
    int loops = 0;
    while (check_game_status(&game) && loops < 50) {
        int bytes = read_game_ipc_events(&game, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
        }
        usleep(10000); /* 10ms */
        loops++;
    }

    assert(!game.is_running);
    assert(game.exit_code == 0);

    game_process_cleanup(&game);
    printf("  -> PASS\n");
}

static void test_launch_game_and_terminate(void) {
    printf("[TEST] Running test_launch_game_and_terminate...\n");
    GameProcess game;
    game_process_init(&game);

    char *const argv[] = { "./game", "--ticks", "100", "--interval", "100", NULL };
    int rc = launch_game(&game, "./game", argv);
    assert(rc == 0);
    assert(game.pid > 0);
    assert(game.is_running);

    /* Allow it to start */
    usleep(20000); /* 20ms */

    /* Terminate the game using safe terminate_game() */
    rc = terminate_game(&game, SIGTERM);
    assert(rc == 0);
    assert(!game.is_running);
    assert(!check_game_status(&game));

    game_process_cleanup(&game);
    printf("  -> PASS\n");
}

int main(void) {
    printf("========================================\n");
    printf("      RUNNING GAME LAUNCHER TESTS       \n");
    printf("========================================\n");

    test_game_process_init_and_cleanup();
    test_launch_game_and_wait();
    test_launch_game_and_terminate();

    printf("\nAll game launcher tests passed!\n");
    return 0;
}
