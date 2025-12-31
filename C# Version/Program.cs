using System;
using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Threading.Tasks;
using Raylib_cs;

class Program
{
    static HttpClient httpClient = new HttpClient();
    static string cachedSong = "";
    static string cachedArtist = "";
    static string cachedAlbumArt = "";
    static Texture2D albumTexture;
    static float lastUpdate = 0;
    static bool textureLoaded = false;

    static (string artist, string title, string artUrl) GetSongInfo()
    {
        try
        {
            // Get artist
            var artistInfo = new ProcessStartInfo
            {
                FileName = "/bin/bash",
                Arguments = "-c \"playerctl -p spotify metadata --format='{{ artist }}' 2>/dev/null || echo 'Unknown'\"",
                RedirectStandardOutput = true,
                UseShellExecute = false,
                CreateNoWindow = true
            };
            string artist;
            using (Process p = Process.Start(artistInfo))
            {
                artist = p.StandardOutput.ReadToEnd().Trim();
                p.WaitForExit();
            }

            // Get title
            var titleInfo = new ProcessStartInfo
            {
                FileName = "/bin/bash",
                Arguments = "-c \"playerctl -p spotify metadata --format='{{ title }}' 2>/dev/null || echo 'No media playing'\"",
                RedirectStandardOutput = true,
                UseShellExecute = false,
                CreateNoWindow = true
            };
            string title;
            using (Process p = Process.Start(titleInfo))
            {
                title = p.StandardOutput.ReadToEnd().Trim();
                p.WaitForExit();
            }

            // Get album art URL
            var artInfo = new ProcessStartInfo
            {
                FileName = "/bin/bash",
                Arguments = "-c \"playerctl -p spotify metadata mpris:artUrl 2>/dev/null || echo ''\"",
                RedirectStandardOutput = true,
                UseShellExecute = false,
                CreateNoWindow = true
            };

            string artUrl;
            using (Process p = Process.Start(artInfo))
            {
                artUrl = p.StandardOutput.ReadToEnd().Trim();
                p.WaitForExit();
            }

            return (artist, title, artUrl);
        }
        catch (Exception)
        {
            return ("Unknown", "No media playing", "");
        }
    }

    static string pendingTexturePath = "";
    static bool isDarkMode = true;

    static async Task DownloadAlbumArt(string url)
    {
        try
        {
            if (string.IsNullOrEmpty(url)) return;

            byte[] imageData = await httpClient.GetByteArrayAsync(url);
            string tempPath = Path.Combine(Path.GetTempPath(), "spotify_album_" + Guid.NewGuid().ToString() + ".jpg");
            await File.WriteAllBytesAsync(tempPath, imageData);

            pendingTexturePath = tempPath;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to download album art: {ex.Message}");
            pendingTexturePath = "";
        }
    }

    static void Main(string[] args)
    {
        Raylib.InitWindow(600, 200, "Tuner");
        Raylib.SetTargetFPS(60);
        Raylib.SetWindowIcon(Raylib.LoadImage("icon.png"));


        bool albumArtLoading = false;

        while (!Raylib.WindowShouldClose())
        {
            float currentTime = (float)Raylib.GetTime();

            // Load pending texture on main thread
            if (!string.IsNullOrEmpty(pendingTexturePath))
            {
                try
                {
                    // Unload previous texture if exists
                    if (textureLoaded)
                    {
                        Raylib.UnloadTexture(albumTexture);
                    }

                    albumTexture = Raylib.LoadTexture(pendingTexturePath);
                    textureLoaded = true;

                    File.Delete(pendingTexturePath);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Failed to load texture: {ex.Message}");
                    textureLoaded = false;
                }
                pendingTexturePath = "";
            }

            // Update song info every 2 seconds
            if (currentTime - lastUpdate > 2.0f)
            {
                var (artist, title, artUrl) = GetSongInfo();

                if (cachedSong != title || cachedArtist != artist)
                {
                    cachedSong = title;
                    cachedArtist = artist;

                    // Download new album art if URL changed
                    if (cachedAlbumArt != artUrl && !string.IsNullOrEmpty(artUrl))
                    {
                        cachedAlbumArt = artUrl;
                        if (!albumArtLoading)
                        {
                            albumArtLoading = true;
                            _ = DownloadAlbumArt(artUrl).ContinueWith(_ => albumArtLoading = false);
                        }
                    }
                }

                lastUpdate = currentTime;
            }

            Raylib.BeginDrawing();

            // Theme colors
            Color bgColor1, bgColor2, textPrimary, textSecondary, placeholderBg, placeholderBorder, placeholderIcon, buttonBg, buttonHover;

            if (isDarkMode)
            {
                bgColor1 = new Color(30, 30, 35, 255);
                bgColor2 = new Color(15, 15, 20, 255);
                textPrimary = new Color(240, 240, 245, 255);
                textSecondary = new Color(160, 160, 170, 255);
                placeholderBg = new Color(40, 40, 45, 255);
                placeholderBorder = new Color(80, 80, 85, 255);
                placeholderIcon = new Color(100, 100, 105, 255);
                buttonBg = new Color(50, 50, 55, 255);
                buttonHover = new Color(70, 70, 75, 255);
            }
            else
            {
                bgColor1 = new Color(245, 245, 250, 255);
                bgColor2 = new Color(230, 230, 240, 255);
                textPrimary = new Color(20, 20, 25, 255);
                textSecondary = new Color(80, 80, 90, 255);
                placeholderBg = new Color(220, 220, 230, 255);
                placeholderBorder = new Color(180, 180, 190, 255);
                placeholderIcon = new Color(150, 150, 160, 255);
                buttonBg = new Color(210, 210, 220, 255);
                buttonHover = new Color(190, 190, 200, 255);
            }

            // Background gradient
            Raylib.ClearBackground(new Color(18, 18, 18, 255));
            Raylib.DrawRectangleGradientV(0, 0, 600, 200, bgColor1, bgColor2);

            int leftMargin = 20;
            int albumSize = 160;

            // Draw album art with shadow
            if (textureLoaded)
            {
                // Shadow
                Raylib.DrawRectangle(leftMargin + 3, 23, albumSize, albumSize, new Color(0, 0, 0, 100));

                // Album art
                Raylib.DrawTextureEx(albumTexture,
                    new System.Numerics.Vector2(leftMargin, 20),
                    0,
                    (float)albumSize / albumTexture.Width,
                    Color.White);

                // Border
                Raylib.DrawRectangleLines(leftMargin, 20, albumSize, albumSize, new Color(255, 255, 255, 30));
            }
            else
            {
                // Placeholder album art
                Raylib.DrawRectangle(leftMargin, 20, albumSize, albumSize, placeholderBg);
                Raylib.DrawRectangleLines(leftMargin, 20, albumSize, albumSize, placeholderBorder);
                Raylib.DrawText("♪", leftMargin + 60, 80, 60, placeholderIcon);
            }

            int textX = leftMargin + albumSize + 25;

            // Draw song title
            string displayTitle = cachedSong.Length > 30 ? cachedSong.Substring(0, 27) + "..." : cachedSong;
            Raylib.DrawText(displayTitle, textX, 50, 28, textPrimary);

            // Draw artist
            string displayArtist = cachedArtist.Length > 35 ? cachedArtist.Substring(0, 32) + "..." : cachedArtist;
            Raylib.DrawText(displayArtist, textX, 85, 20, textSecondary);

            // Animated equalizer bars
            for (int i = 0; i < 5; i++)
            {
                float barHeight = 10 + (float)Math.Abs(Math.Sin(currentTime * 3 + i * 0.5)) * 20;
                Raylib.DrawRectangle(textX + i * 8, (int)(155 - barHeight), 5, (int)barHeight,
                    new Color(100, 200, 100, 255));
            }

            // Theme toggle button in bottom right
            int buttonSize = 40;
            int buttonX = 600 - buttonSize - 15;
            int buttonY = 200 - buttonSize - 15;

            var mousePos = Raylib.GetMousePosition();
            bool isHovering = mousePos.X >= buttonX && mousePos.X <= buttonX + buttonSize &&
                             mousePos.Y >= buttonY && mousePos.Y <= buttonY + buttonSize;

            Color currentButtonColor = isHovering ? buttonHover : buttonBg;

            // Draw button background
            Raylib.DrawRectangle(buttonX, buttonY, buttonSize, buttonSize, currentButtonColor);
            Raylib.DrawRectangleLines(buttonX, buttonY, buttonSize, buttonSize,
                isDarkMode ? new Color(100, 100, 110, 255) : new Color(160, 160, 170, 255));

            // Draw icon (sun or moon)
            int iconCenterX = buttonX + buttonSize / 2;
            int iconCenterY = buttonY + buttonSize / 2;

            if (isDarkMode)
            {
                // Draw sun icon
                Raylib.DrawCircle(iconCenterX, iconCenterY, 8, new Color(255, 220, 100, 255));
                for (int i = 0; i < 8; i++)
                {
                    float angle = i * 45 * (float)Math.PI / 180;
                    int x1 = iconCenterX + (int)(Math.Cos(angle) * 10);
                    int y1 = iconCenterY + (int)(Math.Sin(angle) * 10);
                    int x2 = iconCenterX + (int)(Math.Cos(angle) * 14);
                    int y2 = iconCenterY + (int)(Math.Sin(angle) * 14);
                    Raylib.DrawLineEx(new System.Numerics.Vector2(x1, y1),
                                     new System.Numerics.Vector2(x2, y2),
                                     2, new Color(255, 220, 100, 255));
                }
            }
            else
            {
                // Draw moon icon
                Raylib.DrawCircle(iconCenterX - 2, iconCenterY - 2, 10, new Color(100, 120, 180, 255));
                Raylib.DrawCircle(iconCenterX + 3, iconCenterY - 2, 10, bgColor1);
            }

            // Handle button click
            if (isHovering && Raylib.IsMouseButtonPressed(MouseButton.Left))
            {
                isDarkMode = !isDarkMode;
            }

            Raylib.EndDrawing();
        }

        if (textureLoaded)
        {
            Raylib.UnloadTexture(albumTexture);
        }
        Raylib.CloseWindow();
    }
}
