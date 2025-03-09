

void Update(float dt)
{
    // if (Input::IsKeyPressed(Key::L))
    // {
    //     sunDir = -camera.getForward();
    // }

    if (!Input::IsMouseCaptured() && Input::IsMouseButtonPressed(MouseButton::Left))
    {
        Input::CaptureMouse(true);
    }

    if (Input::IsMouseCaptured() && Input::IsKeyPressed(Key::Escape))
    {
        Input::CaptureMouse(false);
    }

    // if (Input::IsMouseCaptured())
    // {
    //     camera.processMouseMovement(-Input::GetMouseDelta(), true);
    //     camera.processKeyboard(0.01);
    // }

}
