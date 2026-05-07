#include "UIManager.h"

UIManager::UIManager()
{
}

UIManager::~UIManager()
{
    Clear();
}

void UIManager::AddUI(std::shared_ptr<UIBase> ui)
{
    if (ui)
    {
        m_uiElements.push_back(ui);
    }
}

void UIManager::Init()
{
    for (auto& ui : m_uiElements)
    {
        ui->Init();
    }
}

void UIManager::Update(float deltaTime)
{
    for (auto& ui : m_uiElements)
    {
        if (ui->IsVisible())
        {
            ui->Update(deltaTime);
        }
    }
}

void UIManager::Draw()
{
    for (auto& ui : m_uiElements)
    {
        if (ui->IsVisible())
        {
            ui->Draw();
        }
    }
}

void UIManager::Clear()
{
    m_uiElements.clear();
}
