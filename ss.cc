#include <emscripten.h>
#include <iostream>
#include <cmath>

#include <html5.h>

#include "../../KinEmAtic/cogs/SlideShow.h"

emk::SlideShow show;

extern "C" int emkMain()
{
  // Setup background for all images.
  int stage_w = show.Stage().GetWidth() - 12;
  int stage_h = show.Stage().GetHeight() - 12;  
  show.BuildRect("white_bg", emk::Point(5, 5), stage_w, stage_h, "white", "black", 3);
  show.AddBackground(show.Rect());  

  // ---=== TITLE Slide ===---
  show.NewSlide(); // Create the title slide.
  show << show(0.1, 0.08) << emk::Font(stage_w/15) << "["
       << emk::Color("blue") << "Digital"
       << emk::Color("black") << "] Evolution in Action";

  show << show(0.45, 0.35) << emk::Font(stage_w/25) << "Charles Ofria\nMichigan State University";
  show << show(0.45, 0.50) << emk::Font(stage_w/35) << "Professor of Computer Science\nDeputy Director, BEACON Center";

  const int logo_size = show.ScaleY(0.6);
  show << emk::Image("images/BeaconLogo.png", show(0.03, 0.35), logo_size, logo_size);


  // ---=== My Thought Process ===---
  show.NewSlide("My thought processs...");

  show.Pause();

  const int thought_scale = show.ScaleY(0.30);
  show << emk::Image("images/LostInSpace.jpg", show(0.1, 0.2), thought_scale, thought_scale);
  show << emk::Image("images/HAL_eye.jpg", show.Image().GetUR(3,0), thought_scale, thought_scale);
  show << show(0.14, 0.52) << emk::Font(stage_w/35) << "Artificial Intelligence";

  show.Pause();

  show << emk::Image("images/Darwin.jpg", show(0.56, 0.2), thought_scale, thought_scale);
  show << emk::Image("images/Finches.jpg", show.Image().GetUR(3,0), thought_scale, thought_scale);
  show << show(0.66, 0.52) << emk::Font(stage_w/35) << "Evolution";

  show.Pause();
  show << emk::Tween(show.Text(), 3).SetXY( show(0.8, 0.88) ).SetScale(5.0).SetRotation(900).SetEasing(emk::Tween::Linear);

  show.Pause();

  const int shadow_scale = show.ScaleY(0.4);
  show << show.BuildImage("image_descent", "images/DescentShadow.jpg", show(0.2, 0.58), shadow_scale*2.73, shadow_scale);



  // ---=== Evolution is Hard to Study! ===---


  show.NewSlide("Evolution is hard to study!");


  emk::Rect & rect = show.BuildRect("test", emk::Point(50, 50), 100, 100);
  show.Appear(rect);

  show.Pause();

  emk::Rect & rect2 = show.BuildRect("test2", emk::Point(250, 250), 100, 100, "red", "black", 5, true);
  show.Appear(rect2);

  show.NewSlide();

  emk::Rect & rect3 = show.BuildRect("test3", emk::Point(200, 200), 100, 100, "blue", "black", 5, true);
  show.Appear(rect3);

  show.Pause();
  show.Appear(rect);

  show.Start();

  return 0;
}
