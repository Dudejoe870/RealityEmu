#pragma once

void on_vi(void);

int  window_init  (int width, int height, char* title);
int  window_run   (void);
void window_cleanup(void);