/*!
    @file src/Utils/Arena.hpp
    @brief 线性分配内存池，支持非平凡析构对象的自动清理
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <type_traits>

namespace Fig
{
    class Arena
    {
    private:
        struct DestructorNode
        {
            void (*destructor)(void *);
            void          *object;
            DestructorNode *next;
        };

        static constexpr std::size_t CHUNK_SIZE = 64 * 1024; // 64KB 块大小

        std::vector<char *> chunks;
        char               *currentPtr     = nullptr;
        std::size_t         remaining      = 0;
        DestructorNode     *destructorHead = nullptr;

    public:
        Arena() = default;

        ~Arena()
        {
            // 1. 逆序调用析构函数
            DestructorNode *node = destructorHead;
            while (node)
            {
                node->destructor(node->object);
                node = node->next;
            }

            // 2. 释放所有分配的内存块
            for (char *chunk : chunks)
            {
                delete[] chunk;
            }
        }

        // 禁止拷贝和移动，防止内存所有权混乱
        Arena(const Arena &)            = delete;
        Arena &operator=(const Arena &) = delete;

        template <typename T, typename... Args>
        T *Allocate(Args &&...args)
        {
            std::size_t size      = sizeof(T);
            std::size_t alignment = alignof(T);

            // 在当前块中尝试对齐并分配
            void *ptr = allocateRaw(size, alignment);

            // 在分配的内存上构造对象
            T *obj = new (ptr) T(std::forward<Args>(args)...);

            // 如果 T 需要析构（如包含 String），注册到销毁链表
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                // 注意: DestructorNode 本身是 POD，直接在 Arena 里分配，不需要注册析构
                void           *nodeRaw = allocateRaw(sizeof(DestructorNode), alignof(DestructorNode));
                DestructorNode *node    = new (nodeRaw) DestructorNode();

                node->object     = obj;
                node->destructor = [](void *p) { static_cast<T *>(p)->~T(); };
                node->next       = destructorHead;
                destructorHead   = node;
            }

            return obj;
        }

    private:
        void *allocateRaw(std::size_t size, std::size_t alignment)
        {
            // 对齐计算
            std::size_t adjustment = 0;
            std::size_t currentAddr = reinterpret_cast<std::size_t>(currentPtr);
            if (alignment > 0 && (currentAddr % alignment) != 0)
            {
                adjustment = alignment - (currentAddr % alignment);
            }

            if (remaining < (size + adjustment))
            {
                // 当前块空间不足，分配新块
                std::size_t nextChunkSize = (size > CHUNK_SIZE) ? size : CHUNK_SIZE;
                currentPtr                = new char[nextChunkSize];
                chunks.push_back(currentPtr);
                remaining = nextChunkSize;
                adjustment = 0; // 新分配的块通常是最大对齐的
            }

            currentPtr += adjustment;
            void *res = currentPtr;
            currentPtr += size;
            remaining -= (size + adjustment);

            return res;
        }
    };
} // namespace Fig
