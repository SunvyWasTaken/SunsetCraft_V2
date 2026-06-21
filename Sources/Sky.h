//
// Created by sunvy on 21/06/2026.
//

#pragma once


namespace Sunset
{
    class Drawable;
}

class SkyCubed final
{
public:
    SkyCubed();
    ~SkyCubed();
    void Update(const float deltaTime);
    void Draw() const;
private:
    std::unique_ptr<Sunset::Drawable> m_Drawable;
};
