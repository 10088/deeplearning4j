/*
 *  ******************************************************************************
 *  *
 *  *
 *  * This program and the accompanying materials are made available under the
 *  * terms of the Apache License, Version 2.0 which is available at
 *  * https://www.apache.org/licenses/LICENSE-2.0.
 *  *
 *  *  See the NOTICE file distributed with this work for additional
 *  *  information regarding copyright ownership.
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  * License for the specific language governing permissions and limitations
 *  * under the License.
 *  *
 *  * SPDX-License-Identifier: Apache-2.0
 *  *****************************************************************************
 */

package org.deeplearning4j.models.embeddings.learning;

import org.deeplearning4j.models.embeddings.WeightLookupTable;
import org.deeplearning4j.models.embeddings.learning.impl.elements.BatchSequences;
import org.deeplearning4j.models.embeddings.loader.VectorsConfiguration;
import org.deeplearning4j.models.sequencevectors.interfaces.SequenceIterator;
import org.deeplearning4j.models.sequencevectors.sequence.Sequence;
import org.deeplearning4j.models.sequencevectors.sequence.SequenceElement;
import org.deeplearning4j.models.word2vec.wordstore.VocabCache;
import org.nd4j.linalg.api.memory.conf.WorkspaceConfiguration;
import org.nd4j.linalg.api.memory.enums.AllocationPolicy;
import org.nd4j.linalg.api.memory.enums.LearningPolicy;
import org.nd4j.linalg.api.memory.enums.ResetPolicy;
import org.nd4j.linalg.api.memory.enums.SpillPolicy;
import org.nd4j.linalg.api.ndarray.INDArray;

import java.util.concurrent.atomic.AtomicLong;

public interface ElementsLearningAlgorithm<T extends SequenceElement> {
    default WorkspaceConfiguration workspaceConfig() {
       return  WorkspaceConfiguration.builder()
                .initialSize(0)
                .overallocationLimit(0.02)
                .policyLearning(LearningPolicy.OVER_TIME)
                .cyclesBeforeInitialization(2)
                .policyReset(ResetPolicy.BLOCK_LEFT)
                .policySpill(SpillPolicy.REALLOCATE)
                .policyAllocation(AllocationPolicy.OVERALLOCATE)
                .build();
    }

    String getCodeName();

    void configure(VocabCache<T> vocabCache, WeightLookupTable<T> lookupTable, VectorsConfiguration configuration);

    void pretrain(SequenceIterator<T> iterator);

    /**
     * This method does training over the sequence of elements passed into it
     *
     * @param sequence
     * @param nextRandom
     * @param learningRate
     * @return average score for this sequence
     */
    double learnSequence(Sequence<T> sequence, AtomicLong nextRandom, double learningRate);


    boolean isEarlyTerminationHit();

    void finish();

    void finish(INDArray inferenceVector);
}
