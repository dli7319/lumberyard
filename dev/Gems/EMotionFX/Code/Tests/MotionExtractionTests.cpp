/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <EMotionFX/Source/AnimGraph.h>
#include <EMotionFX/Source/AnimGraphNode.h>
#include <EMotionFX/Source/AnimGraphMotionNode.h>
#include <EMotionFX/Source/AnimGraphStateMachine.h>
#include <EMotionFX/Source/BlendTree.h>
#include <EMotionFX/Source/EMotionFXManager.h>
#include <EMotionFX/Source/Importer/Importer.h>
#include <EMotionFX/Source/Motion.h>
#include <EMotionFX/Source/MotionInstance.h>
#include <EMotionFX/Source/MotionSet.h>
#include <EMotionFX/Source/Node.h>
#include <EMotionFX/Source/Skeleton.h>
#include <EMotionFX/Source/SkeletalMotion.h>
#include <EMotionFX/Source/TransformData.h>
#include <Tests/JackGraphFixture.h>

namespace EMotionFX
{
    struct MotionExtractionTestsData
    {
        std::vector<float> durationMultipliers;
        std::vector<AZ::u32> numOfLoops;
    };

    std::vector< MotionExtractionTestsData> motionExtractionTestData
    {
        {
            {0.001f, 0.01f, 1.0f},
            {1000, 100, 1}
        }
    };

    class MotionExtractionFixture
        : public JackGraphFixture
        , public ::testing::WithParamInterface<testing::tuple<bool, MotionExtractionTestsData>>
    {
    public:
        void ConstructGraph() override
        {
            JackGraphFixture::ConstructGraph();
            m_jackSkeleton = m_actor->GetSkeleton();
            m_actorInstance->SetMotionExtractionEnabled(true);
            m_actor->AutoSetMotionExtractionNode();
            m_reverse = testing::get<0>(GetParam());
            m_param = testing::get<1>(GetParam());

            rootNode = m_jackSkeleton->FindNodeAndIndexByName("jack_root", m_jack_rootIndex);
            hipNode = m_jackSkeleton->FindNodeAndIndexByName("Bip01__pelvis", m_jack_hipIndex);
            m_jackPose = m_actorInstance->GetTransformData()->GetCurrentPose();
            AddMotionEntry("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",
                "jack_walk_forward_aim_zup");

            /*
            +------------+       +---------+
            |m_motionNode+------>+finalNode|
            +------------+       +---------+
            */
            m_motionNode = aznew AnimGraphMotionNode();
            BlendTreeFinalNode* finalNode = aznew BlendTreeFinalNode();
            m_motionNode->AddMotionId("jack_walk_forward_aim_zup");
            m_motionNode->SetLoop(true);
            m_motionNode->SetMotionExtraction(true);
            
            m_blendTree = aznew BlendTree();
            m_blendTree->AddChildNode(m_motionNode);
            m_blendTree->AddChildNode(finalNode);
            m_animGraph->GetRootStateMachine()->AddChildNode(m_blendTree);
            m_animGraph->GetRootStateMachine()->SetEntryState(m_blendTree);
            finalNode->AddConnection(m_motionNode, AnimGraphMotionNode::OUTPUTPORT_POSE, BlendTreeFinalNode::INPUTPORT_POSE);
        }

        void AddMotionEntry(const AZStd::string& base64MotionData, const AZStd::string& motionId)
        {
            AZStd::vector<AZ::u8> skeletalMotiondata;
            AzFramework::StringFunc::Base64::Decode(skeletalMotiondata, base64MotionData.c_str(), base64MotionData.size());
            Motion* newMotion = EMotionFX::GetImporter().LoadSkeletalMotion(skeletalMotiondata.begin(), skeletalMotiondata.size());
            EMotionFX::MotionSet::MotionEntry* newMotionEntry = aznew EMotionFX::MotionSet::MotionEntry();
            newMotionEntry->SetMotion(newMotion);
            m_motionSet->AddMotionEntry(newMotionEntry);
            m_motionSet->SetMotionEntryId(newMotionEntry, motionId);
        }

    protected:
        AZ::u32 m_jack_rootIndex = MCORE_INVALIDINDEX32;
        AZ::u32 m_jack_hipIndex = MCORE_INVALIDINDEX32;
        AnimGraphMotionNode* m_motionNode = nullptr;
        BlendTree* m_blendTree = nullptr;
        bool m_reverse = false;
        Node* rootNode = nullptr;
        Node* hipNode = nullptr;
        Pose * m_jackPose = nullptr;
        Skeleton* m_jackSkeleton = nullptr;
        MotionExtractionTestsData m_param;
    };

    TEST_P(MotionExtractionFixture, ReverseRotationMotionExtractionOutputsCorrectDelta)
    {
        // Test motion extraction with reverse effect on and off, rotation to 90 degrees left and right
        m_motionNode->FindMotionInstance(m_animGraphInstance)->SetMotionExtractionEnabled(true);
        m_motionNode->SetReverse(m_reverse);
        GetEMotionFX().Update(0.0f);
        EXPECT_TRUE(m_motionNode->GetIsMotionExtraction()) << "Motion node should use motion extraction effect.";
        EXPECT_NE(m_actor->GetMotionExtractionNode(), nullptr) << "Actor's motion extraction node should not be nullptr.";

        // The expected delta used is the distance of the jack walk forward motion will move in 1 complete duration
        const float expectedDelta = 1.627f;
        for (AZ::u32 paramIndex = 0; paramIndex < m_param.durationMultipliers.size(); paramIndex++)
        {
            // Test motion extraction under different durations/time deltas
            const float motionDuration = 1.066f * m_param.durationMultipliers[paramIndex];
            const float originalPositionY = m_actorInstance->GetWorldSpaceTransform().mPosition.GetY();
            for (AZ::u32 i = 0; i < m_param.numOfLoops[paramIndex]; i++)
            {
                GetEMotionFX().Update(motionDuration);
            }
            const float updatedPositionY = m_actorInstance->GetWorldSpaceTransform().mPosition.GetY();
            const float actualDeltaY = AZ::GetAbs(updatedPositionY - originalPositionY);
            EXPECT_TRUE(AZ::GetAbs(actualDeltaY - expectedDelta) < 0.001f)
                << "The absolute difference between actual delta and expected delta of Y-axis should be less than 0.001f.";
        }

        // Test motion extraction with rotation
        const AZ::Quaternion actorRotation(0.0f, 0.0f, -1.0f, 1.0f);
        m_actorInstance->SetLocalSpaceRotation(actorRotation.GetNormalized());
        GetEMotionFX().Update(0.0f);
        for (AZ::u32 paramIndex = 0; paramIndex < m_param.durationMultipliers.size(); paramIndex++)
        {
            const float motionDuration = 1.066f * m_param.durationMultipliers[paramIndex];
            const float originalPositionX = m_actorInstance->GetWorldSpaceTransform().mPosition.GetX();
            for (AZ::u32 i = 0; i < m_param.numOfLoops[paramIndex]; i++)
            {
                GetEMotionFX().Update(motionDuration);
            }
            const float updatedPositionX = m_actorInstance->GetWorldSpaceTransform().mPosition.GetX();
            const float actualDeltaX = AZ::GetAbs(updatedPositionX - originalPositionX);
            EXPECT_TRUE(AZ::GetAbs(actualDeltaX - expectedDelta) < 0.001f)
                << "The absolute difference between actual delta and expected delta of X-axis should be less than 0.001f.";
        }
    }

    TEST_P(MotionExtractionFixture, DiagonalRotationMotionExtractionOutputsCorrectDelta)
    {
        // Test motion extraction with diagonal rotation
        m_motionNode->FindMotionInstance(m_animGraphInstance)->SetMotionExtractionEnabled(true);
        GetEMotionFX().Update(0.0f);

        const float expectedDeltaX = 1.30162f;
        const float expectedDeltaY = 0.97622f;

        // Use m_reverse to decide rotating diagonally to the left(0.5) or right(-0.5)
        const AZ::Quaternion diagonalRotation = m_reverse ? AZ::Quaternion(0.0f, 0.0f, 0.5f, 1.0f) : AZ::Quaternion(0.0f, 0.0f, -0.5f, 1.0f);
        m_actorInstance->SetLocalSpaceRotation(diagonalRotation.GetNormalized());
        GetEMotionFX().Update(0.0f);
        for (AZ::u32 paramIndex = 0; paramIndex < m_param.durationMultipliers.size(); paramIndex++)
        {
            const float originalPositionX = m_actorInstance->GetWorldSpaceTransform().mPosition.GetX();
            const float originalPositionY = m_actorInstance->GetWorldSpaceTransform().mPosition.GetY();
            const float motionDuration = 1.066f * m_param.durationMultipliers[paramIndex];
            for (AZ::u32 i = 0; i < m_param.numOfLoops[paramIndex]; i++)
            {
                GetEMotionFX().Update(motionDuration);
            }
            const float updatedPositionX = m_actorInstance->GetWorldSpaceTransform().mPosition.GetX();
            const float updatedPositionY = m_actorInstance->GetWorldSpaceTransform().mPosition.GetY();
            const float actualDeltaX = AZ::GetAbs(updatedPositionX - originalPositionX);
            const float actualDeltaY = AZ::GetAbs(updatedPositionY - originalPositionY);
            EXPECT_TRUE(AZ::GetAbs(actualDeltaX - expectedDeltaX) < 0.001f)
                << "Diagonal Rotation: The absolute difference between actual delta and expected delta of X-axis should be less than 0.001f.";
            EXPECT_TRUE(AZ::GetAbs(actualDeltaY - expectedDeltaY) < 0.001f)
                << "Diagonal Rotation: The absolute difference between actual delta and expected delta of Y-axis should be less than 0.001f.";
        }
    }
    
    INSTANTIATE_TEST_CASE_P(MotionExtraction_OutputTests,
        MotionExtractionFixture,
        ::testing::Combine(
            ::testing::Bool(),
            ::testing::ValuesIn(motionExtractionTestData)
        )
    );
    
} // end namespace EMotionFX
