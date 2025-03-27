# Platformer Game - README

## Giới thiệu

Đây là một trò chơi platformer 2D đơn giản được xây dựng bằng thư viện SDL2. Người chơi điều khiển một nhân vật, di chuyển qua các màn chơi, tránh né kẻ thù, bắn đạn và chiến đấu với boss để giành chiến thắng.
## CHƠI NGAY VÀ LUÔN 
*   Ở mục Releases tải file.zip có tên "File zip for play !" về máy.
*   Extract file và sau đó tìm file.exe có tên ".A Game Start." trong folder Game Play để chơi trực tiếp (Nếu không muốn tải CodeBlocks hay các thư viện SDL_2).
*   Còn không thì cài đặt theo các mục dưới.
## Cấu trúc Project
    * Gồm project chính: emday và các file con .h và .cpp
## Yêu cầu hệ thống

*   **Hệ điều hành:** Bất kỳ hệ điều hành nào hỗ trợ SDL2 (Windows, macOS, Linux)
*   **Thư viện:**
    *   SDL2
    *   SDL2\_image
    *   SDL2\_mixer
    *   SDL2\_ttf
*   **Trình biên dịch:** C++ Compiler (g++ hoặc tương đương)

## Hướng dẫn cài đặt và chạy

1.  **Cài đặt các thư viện SDL2:**

    *   **Windows:**
        *   Tải các thư viện SDL2, SDL2\_image, SDL2\_mixer, SDL2\_ttf từ trang web chính thức của SDL.
        *   Giải nén các file đã tải vào một thư mục.
        *   Sao chép các file `.dll` vào thư mục chứa file thực thi (`.exe`) của trò chơi.
        *   Thêm thư mục `include` của các thư viện vào đường dẫn include của trình biên dịch.
        *   Thêm thư mục `lib` của các thư viện vào đường dẫn lib của trình biên dịch.
    *   **macOS (sử dụng Homebrew):**
        ```bash
        brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf
        ```
    *   **Linux (ví dụ, Ubuntu/Debian):**
        ```bash
        sudo apt-get update
        sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
        ```

2.  **Biên dịch mã nguồn:**

    *   Sử dụng trình biên dịch C++ (ví dụ, g++) để biên dịch file `main.cpp`.  Đảm bảo các thư viện SDL2 được liên kết trong quá trình biên dịch.
    *   Ví dụ sử dụng g++:
        ```bash
        g++ main.cpp -o platformer `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf
        ```
        (Lưu ý: Lệnh này có thể khác tùy thuộc vào hệ điều hành và cách cài đặt SDL2.)

3.  **Chạy trò chơi:**

    *   Sau khi biên dịch thành công, file thực thi `platformer` (hoặc tên bạn đặt) sẽ được tạo ra.
    *   Chạy file thực thi này để bắt đầu trò chơi. Đảm bảo các tài nguyên (hình ảnh, âm thanh, font) nằm trong cùng thư mục với file thực thi.

## Hướng dẫn chơi

*   **Điều khiển:**
    *   **Phím mũi tên trái:** Di chuyển sang trái
    *   **Phím mũi tên phải:** Di chuyển sang phải
    *   **Phím mũi tên lên:** Nhảy
    *   **Phím Space:** Bắn đạn
*   **Mục tiêu:**
    *   Di chuyển qua màn chơi, tránh né kẻ thù và chướng ngại vật.
    *   Bắn hạ kẻ thù.
    *   Tiêu diệt boss ở cuối màn chơi.
    *   Chiến thắng trò chơi sau khi tiêu diệt đủ số lượng boss yêu cầu.
*   **Cách chơi:**
    *   Bạn bắt đầu ở màn hình chờ. Click vào nút "Start" để bắt đầu trò chơi.
    *   Người chơi có thể di chuyển sang trái, phải, nhảy và bắn đạn để tiêu diệt kẻ thù.
    *   Đầu tiên bạn phải nhảy vượt qua 1 số chướng ngại vật.
    *   Khi rơi xuống thì trò chơi kết thúc.
    *   Khi chạm vào kẻ thù, trò chơi sẽ kết thúc.
    *   Sau khi tiêu diệt khoảng 12 enemy con thì bức tường sẽ mở ra.
    *   Khi tường mở, bạn phải đi qua tường để gặp boss, tránh boss bằng cách nhảy.
    *   Tiêu diệt boss bằng cách bắn đạn để tiếp tục.
    *   Sau khi tiêu diệt đủ 4 boss, bạn sẽ thắng trò chơi.
    *   Khi thua hoặc thắng, bạn sẽ được đưa đến màn hình game over. Tại đây, bạn có thể chọn "Restart" để chơi lại hoặc "Quit" để thoát game.

## Cấu trúc dự án
## Ghi chú

*   File `.ttf`, `.png`, `.mp3`, `.wav` phải nằm trong cùng thư mục với file thực thi.
*   Trò chơi có thể được tùy chỉnh bằng cách thay đổi các hằng số như `SCREEN_WIDTH`, `SCREEN_HEIGHT`, `TILE_SIZE`, `FRAME_DELAY`, `BOSS_FRAME_DELAY` để điều chỉnh kích thước màn hình, kích thước gạch, tốc độ animation,...
*   Có thể thêm nhiều màn chơi, kẻ thù, boss,... bằng cách chỉnh sửa mã nguồn.
*   Trọng lực của người chơi được điều chỉnh khi boss xuất hiện (giảm trọng lực để tăng độ khó).

## Tác giả
- Phạm Dương Khanh_24020170_IT8
