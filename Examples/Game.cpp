// ImGui Stuff

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Set up the new frame for the UI, then build
	// this frame's interface.  Note that the building
	// of the UI could happen at any point during update.
	PrepareUIFrame(deltaTime);
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	// Update the camera this frame
	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);



		// Draw the UI after everything else
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	
}

// --------------------------------------------------------
// Prepares a new frame for the UI, feeding it fresh
// input and time information for this new frame.
// --------------------------------------------------------
void Game::PrepareUIFrame(float deltaTime)
{
	// Get a reference to our custom input manager
	Input& input = Input::GetInstance();

	// Reset input manager's gui state so we don�t
	// taint our own input
	input.SetKeyboardCapture(false);
	input.SetMouseCapture(false);

	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);
}

// --------------------------------------------------------
// Builds the UI for the current frame
// --------------------------------------------------------
void Game::BuildUI()
{
    bool show_demo_window = true;
	// Should we show the built-in demo window?
	if (show_demo_window)
	{ // Demo window code can be found here: https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp
		ImGui::ShowDemoWindow();
	}

    ImGui::Begin("Menu");
    {
    ImGui::Text("This is some useful text.");
	ImGui::BulletText("Bulleted Text");

	
    static int counter = 0;
	if (ImGui::Button("Button")) 
    { // Most widgets return true when edited/activated
		counter++;
	}
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Button("Button1");
	ImGui::SameLine();
	ImGui::Button("Button2");
	ImGui::SameLine();
	ImGui::Button("Button3");
	ImGui::SameLine();

    ImGui::ArrowButton("arrow", ImGuiDir_Left);

    // Edit 1 float using a slider from 0.0f to 1.0f
	static float f = 0.0f;
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

	// Provide the address of the first element to create a
	// 3-component, draggable editor for a vector
	static XMFLOAT3 vec(10.0f, -2.0f, 35.0f);
	ImGui::DragFloat3("Edit a vector", &vec.x);

	// Edit 3 floats representing a color
	static ImVec4 color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGui::ColorEdit3("color", (float*)&color);

    // Bool stores our window open/close state
	static bool show_another_window = false;
	ImGui::Checkbox("Open a new window", &show_another_window);
	if (show_another_window)
	{
		// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		// if (ImGui::Button("Close Me"))
			// show_another_window = false;
		ImGui::End();
	}

    // Create collapsible sections
	if (ImGui::CollapsingHeader("Collapsing Header"))
	{
		if (ImGui::TreeNode("TreeNode 1"))
		{
			ImGui::Text("Window Dimensions: %i x %i", this->windowWidth, this->windowHeight);
            ImGui::Spacing();
			ImGui::Text("Cursor Position: %f, %f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("TreeNode 2"))
		{
			static ImVec4 color2 = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			ImGui::ColorPicker3("colorPick", (float*)&color2);
			ImGui::TreePop();
		}
	}

    for (int i = 0; i < 3; i++)
	{ // Loop and make dynamic controls
		// PushID() allows the system to create a unique ID
		ImGui::PushID(i);
		if (ImGui::TreeNode("Tree Node", "Node %d", i))
		{
			ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

    	static bool checkbox = true;
	ImGui::Checkbox("checkbox", &checkbox);

	static int e = 0;
	ImGui::RadioButton("radio a", &e, 0);
	ImGui::SameLine();
	ImGui::RadioButton("radio b", &e, 1);
	ImGui::SameLine();
	ImGui::RadioButton("radio c", &e, 2);

	// Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
	for (int i = 0; i < 7; i++)
	{
		if (i > 0)	ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
		ImGui::Button("Click");
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}

	// Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
	ImGui::AlignTextToFramePadding();
	// Arrow buttons with Repeater
	static int counter2 = 0;
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter2--; }
	ImGui::SameLine(0.0f, spacing);
	if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter2++; }
	ImGui::PopButtonRepeat();
	ImGui::SameLine();
	ImGui::Text("%d", counter2);

	ImGui::LabelText("label", "Value");

	// Combo boxes AKA Dropdowns
	if (ImGui::TreeNode("Combos"))
	{
		// Using the generic BeginCombo() API, you have full control over how to display the combo contents.
		// (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
		// stored in the object itself, etc.)
		const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
		static int item_current_idx = 0; // Here we store our selection data as an index.
		const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
		if (ImGui::BeginCombo("combo 1", combo_preview_value))
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				const bool is_selected = (item_current_idx == n);
				if (ImGui::Selectable(items[n], is_selected))
					item_current_idx = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// Simplified one-liner Combo() API, using values packed in a single constant string
		// This is a convenience for when the selection set is small and known at compile-time.
		static int item_current_2 = 0;
		ImGui::Combo("combo 2 (one-liner)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

		// Simplified one-liner Combo() using an array of const char*
		// This is not very useful (may obsolete): prefer using BeginCombo()/EndCombo() for full control.
		static int item_current_3 = -1; // If the selection isn't within 0..count, Combo won't display a preview
		ImGui::Combo("combo 3 (array)", &item_current_3, items, IM_ARRAYSIZE(items));

		// Simplified one-liner Combo() using an accessor function
		struct Funcs { static bool ItemGetter(void* data, int n, const char** out_str) { *out_str = ((const char**)data)[n]; return true; } };
		static int item_current_4 = 0;
		ImGui::Combo("combo 4 (function)", &item_current_4, &Funcs::ItemGetter, items, IM_ARRAYSIZE(items));

		ImGui::TreePop();
	}

    }    
    ImGui::End();
}

void Game::ExanpleStuff(){
    
}