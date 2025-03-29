

void Update()
{
    if (Input::IsKeyPressed(Key::L))
    {
        sunDir = -camera.getForward();
    }

    if (!Input::IsMouseCaptured() && Input::IsMouseButtonPressed(MouseButton::Left))
    {
        Input::CaptureMouse(true);
    }

    if (Input::IsMouseCaptured() && Input::IsKeyPressed(Key::Escape))
    {
        Input::CaptureMouse(false);
    }

    if (Input::IsMouseCaptured())
    {
        camera.speed = 30;
        camera.processMouseMovement(-Input::GetMouseDelta(), true);
        camera.processKeyboard(dt);
    }

}
