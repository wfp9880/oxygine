#pragma once
#include "oxygine-include.h"
#include "RenderDelegate.h"
#include "core/NativeTexture.h"
#include "core/Renderer.h"
#include "math/Rect.h"
#include "tween/Tween.h"

namespace oxygine
{
    class PostProcessOptions
    {
    public:
        enum
        {
            flag_singleR2T = 1,
            flag_fullscreen = 1 << 1,
            flag_screen = 1 << 2,
            flag_fixedBounds = 1 << 3,
        };

        PostProcessOptions(int flags = 0) : _flags(flags), _downscale(1), _clearColor(0, 0, 0, 0), _format(TF_R4G4B4A4) {}
        PostProcessOptions& fullscreen(bool enable = true) { _flags = enable ? (_flags | flag_fullscreen) : (_flags  & (~flag_fullscreen)); return *this; }
        PostProcessOptions& singleRender(bool enable = true) { _flags = enable ? (_flags | flag_singleR2T) : (_flags  & (~flag_singleR2T)); return *this; }
        //loops -(2, 3, 4, ...),  final size: 2^loops
        PostProcessOptions& downscale(int loops = 2) { _downscale = loops; return *this; }
        PostProcessOptions& clear(const Color& c) { _clearColor = c; return *this; }
        PostProcessOptions& fixedBounds(const RectF& b) { _fixedBounds = b; _flags |= flag_fixedBounds; return *this; }
        PostProcessOptions& format(TextureFormat tf) { _format = tf; return *this; }

        int _flags;
        int _downscale;
        RectF _fixedBounds;
        Color _clearColor;
        TextureFormat _format;
    };


    class PostProcess
    {
    public:

        static ShaderProgram* shaderBlurV;
        static ShaderProgram* shaderBlurH;
        static ShaderProgram* shaderBlit;

        static ShaderProgram* shaderBlurV2;
        static ShaderProgram* shaderBlurH2;

        static void initShaders();
        static void freeShaders();

        PostProcess(const PostProcessOptions& opt);
        ~PostProcess();

        void free();

        void update(Actor* actor);
        Rect getScreenRect(const Actor& actor) const;

        Point _extend;
        spNativeTexture _rt;
        TextureFormat _format;
        Transform _transform;
        Rect _screen;

        PostProcessOptions _options;
    };

    class PPTask
    {
    public:
        virtual ~PPTask() {}
        virtual void addRefPP() = 0;
        virtual void releaseRefPP() = 0;
        virtual void renderPP() = 0;
    };

    DECLARE_SMART(TweenPostProcess, spTweenPostProcess);

    class TweenPostProcess : public TweenObj, public RenderDelegate, public PPTask
    {
    public:
        TweenPostProcess(const PostProcessOptions& opt);
        ~TweenPostProcess();

        void renderPP() override;
        void addRefPP() override;
        void releaseRefPP() override;

        void init(Actor& actor) override;
        void update(Actor& actor, float p, const UpdateState& us) override;
        void done(Actor& actor) override;
    protected:
        virtual void _renderPP() {}


        Actor* _actor;
        PostProcess _pp;
        float _progress;

        RenderDelegate* _prevMaterial;
    };


    class RenderTargetsManager
    {
    public:
        RenderTargetsManager();

        spNativeTexture get(spNativeTexture current, int w, int h, TextureFormat tf);
        void update();
        void reset();

    protected:
        void print();
        bool isGood(const spNativeTexture& t, int w, int h, TextureFormat tf) const;

        typedef std::vector<spNativeTexture> rts;
        rts _rts;

        typedef std::vector<spNativeTexture> free;
        free _free;
    };

    void pass(spNativeTexture srcTexture, const Rect& srcRect, spNativeTexture destTexture, const Rect& destRect, const Color& color = Color(Color::White), const Color& clearColor = Color(0));

    RenderTargetsManager& getRTManager();

    void updatePortProcessItems();
    bool isRenderingPostProcessItems();
    void addPostProcessItem(PPTask*);
    void removePostProcessItem(PPTask*);
    void clearPostProcessItems();
    int getNumPostProcessItem();
}
