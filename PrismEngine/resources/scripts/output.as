
// // Example of defining a progress bar component
// // all componenets inherently inherit the Element class in angelscript while contains all the property definitions and stuff;
// // state variables can be shorthand defined in the "constructor" defintition (or in the body like in the PlayPause exmaple)
// // state variables store their value across frames only taking back the value passed in if it changes
// component ProgressBar(state float value, float min = 0, float max = 1)
// {
//     // auto is a variable that is recalculated every frame. It is just plain angelscript.
//     auto range = max - min;

//     Element fill
//     {
//         // properties are basically built in variables related to style. Like an inline css.
//         // however they support full angelscript code inside them and are recalculated every frame.
//         width: (value / range) * 100%; // this is plain math in angelscript except 100% is just 1.0. Maybe in angelscrip there can be a length type that defines the units of the measurment
//         height: 100%;
//     }
// }

// // this inherits the ProgressBar and calls the super function.
// component Slider(float startValue, float min = 0, float max = 1) : ProgressBar(startValue, min, max)
// {
//     // all this is just angelscript
//     if (Pressed) // pressed in an inherent property accessible inside the component context, not sure how this would show up in angelscript API
//     {
//         CaptureMouse(); // capture mouse forces it to keep handling this event even when the mouse is not hovering this element anymore
//         auto elementWidth = Computed.width;
//         auto mouseXRelative = MouseX - Computed.x;
//         auto p = clamp(mouseXRelative / elementWidth, 0.0, 1.0);
//         value = min + p * (max - min); // we are setting the state variable 'value' of the parent class here
//     }
// }

// component Button(string labelText = "")
// {
//     padding: 10px;

//     // every element already has click handling so no need to add explicit support for this in our button component

//     // adds an image and text element with default empty values (except text). The properties of these would have to be set by the user of this component.
//     Image icon; // you can leave off the parenthesis to just initialize an empty object.
//     Text label(labelText); // adds a text element called textEl with the text from the label argument. Modifying this after the component creation will override this value.
// }

// component ToggleIconButton(string onIcon, string offIcon) : Button
// {
//     state bool on = false; // initializes the variable to false which will stay across frames unless changed.

//     // angelscript
//     if (Clicked)
//     {
//         on = !on;
//     }

//     // sets the icon path in the parent
//     iconEl.path = on ? onIcon : offIcon;
// }

// component Column()
// {
//     direction: column;
// }

// component Row()
// {
//     direction: row;
// }

// void DrawRoot()
// {
//     Column // start an actual UI component in the root
//     {
//         Text header("Example") // text is a built in type
//         {
//             font-size: 24px;
//             font-color: black; // black would be a global variable in angelscript probably or it could be something inherent to this UI language
//         }

//         Row
//         {
//             Column left
//             {
//                 width: 250px;
//                 height: fill;
//                 padding: 10px;

//                 Text("Navigation") // again we can leave out the component name if we never reference it later
//                 {
//                     font-color: color(0.78, 0.78, 0.78); // rgb could either be an angelscript global function to create a color object or it could be parsed by our parser and turned into a color object.
//                     font-size: 18px;
//                 }

//                 Button homeButton("Label")
//                 {
//                     background-color: color(0.08, 0.08, 0.08, 0.8); // colors defined in percent notation
//                     if (Hovered)
//                     {
//                         background-color: background-color.alpha(1); // we can access the current value (not final computed value) and modify it
//                     }
//                 }; // create a button with a label and some styles

//                 // angelscript
//                 homeButton.label.text = "Test"; // now we will never see that home button says "Label" even though it is being set every frame

//                 if (homeButton.Clicked) // returns true the frame the mouse is let up while hovering this or while the mouse is captured on this element
//                 {
//                     print("HOME!");
//                 }
//             }

//             Column right
//             {
//                 Slider volume(0.5);
//                 print(volume.value);
//             }
//         }
//     }
// }
// -----------------------------------------------




// ---------- BUILT IN TYPES START ------------------

// these are all EXAMPLES of the built in type definitions NOT full implementations
// These should not be included in the compiled output
enum UISizeUnit
{
    Pixel = 0,
    Percent = 1
}

class UISize
{
    float value;
    UISizeUnit unit;

    UISize(float value, UISizeUnit unit = UISizeUnit::Pixel) // constructor for the UISize class
    {
        this.value = value;
        this.unit = unit;
    }
}

// statics go inside the namespace
namespace UISize
{
    UISize Fill = UISize(0, UISizeUnit::Percent);
    UISize Fit = UISize(0, UISizeUnit::Percent);
    UISize Auto = UISize(-1, UISizeUnit::Pixel);
}

class UISize4
{
    UISize left;
    UISize top;
    UISize right;
    UISize bottom;

    UISize4(float all, UISizeUnit unit = UISizeUnit::Pixel) // constructor for the UISize4 class
    {
        this.left = UISize(all, unit);
        this.top = UISize(all, unit);
        this.right = UISize(all, unit);
        this.bottom = UISize(all, unit);
    }

    UISize4(float left, float top, float right, float bottom, UISizeUnit unit = UISizeUnit::Pixel) // constructor for the UISize4 class
    {
        this.left = UISize(left, unit);
        this.top = UISize(top, unit);
        this.right = UISize(right, unit);
        this.bottom = UISize(bottom, unit);
    }

    UISize4(float vertical, float horizontal, UISizeUnit unit = UISizeUnit::Pixel) // constructor for the UISize4 class
    {
        this.left = UISize(horizontal, unit);
        this.top = UISize(vertical, unit);
        this.right = UISize(horizontal, unit);
        this.bottom = UISize(vertical, unit);
    }
}

class Color4
{
    Color left;
    Color top;
    Color right;
    Color bottom;

    Color4(Color all) // constructor for the Color4 class
    {
        this.left = all;
        this.top = all;
        this.right = all;
        this.bottom = all;
    }

    Color4(Color left, Color top, Color right, Color bottom) // constructor for the Color4 class
    {
        this.left = left;
        this.top = top;
        this.right = right;
        this.bottom = bottom;
    }

    Color4(Color vertical, Color horizontal) // constructor for the Color4 class
    {
        this.left = horizontal;
        this.top = vertical;
        this.right = horizontal;
        this.bottom = vertical;
    }
}

enum Direction
{
    Row = 0,
    Column = 1
}

class Element
{
    UISize fontSize;
    Color fontColor;
    UISize width;
    UISize height;
    UISize4 margin;
    UISize4 padding;
    UISize4 borderWidth;
    Color4 borderColor;
    Color backgroundColor;
    Direction direction;

    bool Clicked;
    bool Pressing;
    bool Hovered;
}

class Image : Element
{
    string path;
    Image()
    {
        this.path = ""; // default to empty string
    }

    Image(string imagePath)
    {
        this.path = imagePath;
    }
}

class Text : Element
{
    string text;
    Text()
    {
        this.text = ""; // default to empty string
    }

    Text(string text)
    {
        this.text = text;
    }
}

class UIBuilder
{
    void begin(Element element) // this is a function that will be called to start the component
    {
        // implementation of the begin function
    }

    void end(Element element) // this is a function that will be called to end the component
    {
        // implementation of the end function
    }
}

UIBuilder ui; // this is a global variable that will be used to call the begin and end functions

// color is already defined in the existing API, but the static colors are not so I am defining them here:

namespace Color
{
    Color Black = Color(0, 0, 0);
    Color White = Color(1, 1, 1);
    Color Red = Color(1, 0, 0);
    Color Green = Color(0, 1, 0);
    Color Blue = Color(0, 0, 1);
    Color Yellow = Color(1, 1, 0);
    Color Cyan = Color(0, 1, 1);
    Color Magenta = Color(1, 0, 1);
}

// --------- BUILT IN TYPES END ------------------

class _ProgressBarState
{
    // this stores the previous value passed into the constructor.
    // This is only used for state variables defined in the constructor
    // we can then check if the new value is different and if it is overwrite the real value state.
    float _value_last;
    // both these state values will be serialized and stored across frames
    float value;
}

class ProgressBar : Element
{
    _ProgressBarState state; // this is the state of the progress bar component and will be saved then loaded between frames
    // non-state parameters are always defined directly after the state variable.
    float min;
    float max;

    // then explicitly defined child elements
    // the element will not be serialized or saved since it is just a child reference not a state variable
    Element fill;

    // define properties for the state variables so they are stored in state but still accessible directly on the object
    float value
    {
        get
        {
            return state.value; // return the value of the state variable
        }
        set
        {
            // this setting doesn't need to check if it is different that the _value_last because it is an explicit set. Whereas a value passed into the constructor is not.
            state.value = value; // set the value of the state variable
        }
    }

    // no property for the _value_last state variable because it is only used internally in the component and not needed outside of it

    float range
    {
        get
        {
            return max - min;
        }
    }

    ProgressBar(float value, float min = 0, float max = 1) // constructor for the ProgressBar component
    {
        // handle constructor defined state variables
        if (value != state._value_last) // check if the value has changed
        {
            this.value = value; // set the value to the new value
            state._value_last = value; // set the last value to the new value
        }

        // initialize children
        fill = Element(); // create a new element for the fill
        fill.width = UISize((this.value / (max - min)) * 100, UISizeUnit::Percent); // set the width of the fill element to the value passed in
        fill.height = UISize(100, UISizeUnit::Percent); // set the height of the fill element to 100%

    }
}

class Slider : ProgressBar
{

    Slider(float startValue, float min = 0, float max = 1)
    {
        super(startValue, min, max); // call the parent constructor

        if (Pressing)
        {
            CaptureMouse();
            auto elementWidth = Computed.width;
            auto mouseXRelative = MouseX - Computed.x;
            auto p = clamp(mouseXRelative / elementWidth, 0.0, 1.0);

            // we are setting the state variable 'value' of the parent class here
            // but because we are setting it after initializing the parent this wont take effect until one frame later
            // how could this be fixed? The original language doesn't define an order for inherited code to run in.
            // maybe we can comehow define direct assignments to properties as functions
            // so in other words if I set fill.width = value * 100; maybe it should actually store a property for that so it can be caluclated later instead of immediately.
            // for now the way it is is fine, one frame behind is usually unnoticeable
            value = min + p * (max - min);
        }
    }
}

class Button : Element
{
    Image icon; // taken from comp def as predefined children
    Text label; // taken from comp def as predefined children
    Button(string labelText = "") // constructor for the button component
    {
        label = Text(labelText);
        icon = Image(); // initialize the icon to an empty image object
        this.padding = UISize4(10); // taken from the component definition
    }
}

class _ToggleIconButtonState
{
    bool on;
}

class ToggleIconButton : Button
{
    _ToggleIconButtonState state; // this is the state of the toggle icon button component and will be saved then loaded between frames

    // define properties for the state variables so they are stored in state but still accessible directly on the object
    bool on
    {
        get
        {
            return state.on; // return the value of the state variable
        }
        set
        {
            state.on = value; // set the value of the state variable
        }
    }

    ToggleIconButton(string onIcon, string offIcon)
    {
        super("");

        // handle constructor defined state variables
    }
}

class Column : Element
{
    Column()
    {
        this.direction = Direction::Column; // this is taken from the component definition
    }
}

class Row : Element
{
    Row()
    {
        this.direction = Direction::Row; // this is taken from the component definition
    }
}

void DrawRoot()
{
    Column _column_1 = Column(); // generate an ID for this component when compiled
    ui.begin(_column_1);

    Text header = Text("Example"); // the header name is given so we use it
    header.fontSize = UISize(24, UISizeUnit::Pixel); // Convert font size into UISize with units
    header.fontColor = Color::Black; // convert our built in colors to static color definitions in angelscript
    ui.begin(header); // begin the header component
    ui.end(header); // end the header component

    Row _row_1 = Row(); // generate an ID for this component when compiled
    ui.begin(_row_1); // begin the row component

    Column _column_2 = Column(); // generate an ID for this component when compiled
    _column_2.width = UISize(250); // set the width of the column to 250 pixels, pixels is default so doesn't have to be included
    _column_2.height = UISize::Fill; // set the height of the column to fill the remaining space
    _column_2.padding = UISize4(10); // set the padding of the column to 10 pixels on all sides
    ui.begin(_column_2); // begin the column component

    Text navigation = Text("Navigation"); // create a text element for the navigation header
    navigation.fontColor = Color(0.78, 0.78, 0.78);
    navigation.fontSize = UISize(18, UISizeUnit::Pixel); // set the font size of the text to 18 pixels
    ui.begin(navigation); // begin the text component
    ui.end(navigation); // end the text component

    Button homeButton = Button("Label"); // create a button with a label
    homeButton.backgroundColor = Color(0.08, 0.08, 0.08, 0.8);
    ui.begin(homeButton); // begin the button component
    ui.end(homeButton); // end the button component

    // there is no way to know where the home button will be accessed after this so we cannot rely on the ui.end to actually calcualte properties of the component
    // instead ui.end only serves to find children and parent relationships. There will be an ending build step that will handle properties for all components if needed

    // this angelscript section is just copied directly
    homeButton.label.text = "Test";

    if (homeButton.Clicked)
    {
        print("HOME!");
    }

    // etc....

}