#include <GLUL/GL++/Context.h>
#include <GLUL/GUI/Container.h>
#include <GLUL/GUI/Checkbox.h>
#include <GLUL/Logger.h>

#include <cctype>


namespace GLUL {

    namespace GUI {

        Checkbox::Checkbox(Container& parent, bool state) : Checkbox(&parent, state) {

        }

        Checkbox::Checkbox(Container* const parent, bool state) : Component(parent), border(*this) {
            setColor(glm::vec4(1.0f));
            setMarkColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            setMarkScale(0.5f);
            setState(state);

            _glInitialized = false;

            onMouseClick += Event::MouseClick::Handler(
                "__GLUL::GUI::Checkbox::MouseClick",
                [&](Component& component, const Event::MouseClick& onMouseClickEvent) {
                    Checkbox& checkbox = *static_cast<Checkbox*>(&component);

                    if(onMouseClickEvent.button == Input::MouseButton::Left)
                        checkbox.switchState();
                }
            );
        }

        Checkbox::~Checkbox() {

        }
        
        Checkbox::operator bool() const {
            return getState();
        }

        const Checkbox& Checkbox::render() const {
            if(isVisible() && getAlpha() > 0.0f) {
                if(!isValid())
                    validate();

                // Render using font
                getProgram().use();

                _vao.bind();
                _vao.drawArrays();
                _vao.unbind();

                getProgram().unbind();
            }

            return *this;
        }

        Checkbox& Checkbox::update(double deltaTime) {
            (void) deltaTime;

            if(!isValid())
                validate();

            return *this;
        }

        const Checkbox& Checkbox::validate() const {
            Checkbox* thisConstless = const_cast<Checkbox*>(this);

            // (Re)build VBO
            GL::VertexBuffer::Data vertexData;
            std::vector<glm::vec4> vertices = getVertices();

            vertexData.data = vertices.data();
            vertexData.size = vertices.size() * sizeof(glm::vec4);
            vertexData.pointers.push_back(GL::VertexAttrib(0, 4, GL_FLOAT, sizeof(glm::vec4) * 2, nullptr));
            vertexData.pointers.push_back(GL::VertexAttrib(1, 4, GL_FLOAT, sizeof(glm::vec4) * 2, sizeof(glm::vec4)));

            _vbo.bind();
                thisConstless->_vbo.setUsage(GL::VertexBuffer::Usage::DynamicDraw);
                thisConstless->_vbo.setData(vertexData);
            _vbo.unbind();

            // Set vertices draw count
            thisConstless->_vao.setDrawCount(vertices.size() / 2);

            // Initialize VAO
            if(_glInitialized == false) {
                thisConstless->_vao.setDrawTarget(GL::VertexArray::DrawTarget::Triangles);

                _vao.bind();
                    thisConstless->_vao.attachVBO(&_vbo);
                    thisConstless->_vao.setAttribPointers();
                _vao.unbind();

                thisConstless->_glInitialized = true;
            }

            thisConstless->setValid();

            return *this;
        }

        const glm::vec4& Checkbox::getColor() const {
            return _color;
        }

        const glm::vec4& Checkbox::getMarkColor() const {
            return _markColor;
        }

        float Checkbox::getMarkScale() const {
            return _markScale;
        }

        float Checkbox::getAlpha() const {
            return _color.a;
        }

        bool Checkbox::getState() const {
            return _state;
        }

        Checkbox& Checkbox::setColor(const glm::vec3& color) {
            setColor(glm::vec4(color, getAlpha()));

            return *this;
        }

        Checkbox& Checkbox::setColor(const glm::vec4& color) {
            _color = color;

            setInvalid();

            return *this;
        }

        Checkbox& Checkbox::setMarkColor(const glm::vec3& color) {
            setMarkColor(glm::vec4(color, getMarkColor().a));

            return *this;
        }

        Checkbox& Checkbox::setMarkColor(const glm::vec4& color) {
            _markColor = color;

            setInvalid();

            return *this;
        }

        Checkbox& Checkbox::setMarkScale(float scale) {
            _markScale = scale;

            setInvalid();

            return *this;
        }

        Checkbox& Checkbox::setAlpha(float alpha) {
            setColor(glm::vec4(getColor().r, getColor().g, getColor().b, alpha));

            return *this;
        }

        Checkbox& Checkbox::setState(bool state) {
            bool oldState = getState();

            _state = state;

            setInvalid();
            validate();

            onValueChange.call(*this, GLUL::GUI::Event::ValueChange<bool>(oldState, getState()));

            return *this;
        }

        Checkbox& Checkbox::setSize(const glm::vec2& size) {
            Component::setSize(size);

            return *this;
        }

        Checkbox& Checkbox::setPosition(const glm::vec2& position) {
            Component::setPosition(position);

            return *this;
        }


        bool Checkbox::switchState() {
            setState(!getState());

            return getState();
        }

        GL::Program& Checkbox::getProgram() {
            static GL::Program program(
                GL::Shader("assets/shaders/GLUL/GUI/Button.vp", GL::Shader::Type::VertexShader), 
                GL::Shader("assets/shaders/GLUL/GUI/Button.fp", GL::Shader::Type::FragmentShader)
            );

            return program;
        }

        std::vector<glm::vec4> Checkbox::getVertices() const {
            std::vector<glm::vec4> result;

            glm::vec2 scrPos = getScreenPosition();
            glm::vec2 posStart = glm::vec2(scrPos.x, GL::Context::Current->getViewportSize().y - scrPos.y);
            glm::vec2 posEnd = posStart + glm::vec2(getSize().x, -getSize().y);

            glm::vec2 borStart = posStart - glm::vec2( border.getOffset(), -border.getOffset());
            glm::vec2 borEnd   = posEnd   - glm::vec2(-border.getOffset(),  border.getOffset());
            glm::vec2 borWidth = glm::vec2(static_cast<float>(border.getWidth()));
            
            // Checkbox
            pushColoredRectangle(result, posStart, posEnd, getColor());

            if(getState() == true) {
                glm::vec2 markDiff = glm::vec2(
                     0.5f * getMarkScale() * getSize().x, 
                    -0.5f * getMarkScale() * getSize().y
                );

                pushColoredRectangle(result, posStart + markDiff, posEnd - markDiff, getMarkColor());
            }

            // Border
            pushColoredRectangle(result, glm::vec2(borStart.x, borStart.y), glm::vec2(borEnd.x, borStart.y - borWidth.y), border.getColor()); // top
            pushColoredRectangle(result, glm::vec2(borStart.x, borStart.y), glm::vec2(borStart.x + borWidth.x, borEnd.y), border.getColor()); // left
            pushColoredRectangle(result, glm::vec2(borEnd.x - borWidth.x, borStart.y), glm::vec2(borEnd.x, borEnd.y), border.getColor()); // right
            pushColoredRectangle(result, glm::vec2(borStart.x, borEnd.y + borWidth.y), glm::vec2(borEnd.x, borEnd.y), border.getColor()); // bottom

            return result;
        }

    }

}
