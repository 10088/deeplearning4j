/* ******************************************************************************
 *
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 *  See the NOTICE file distributed with this work for additional
 *  information regarding copyright ownership.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

package org.nd4j.linalg.jcublas.ops.executioner;

import org.nd4j.linalg.api.memory.Deallocator;
import org.nd4j.linalg.api.memory.ReferenceMetaData;
import org.nd4j.linalg.factory.Nd4j;
import org.nd4j.linalg.profiler.data.eventlogger.EventLogger;
import org.nd4j.linalg.profiler.data.eventlogger.EventType;
import org.nd4j.linalg.profiler.data.eventlogger.LogEvent;
import org.nd4j.linalg.profiler.data.eventlogger.ObjectAllocationType;
import org.nd4j.nativeblas.NativeOpsHolder;
import org.nd4j.nativeblas.OpaqueContext;

public class CudaOpContextDeallocator implements Deallocator {
    private transient final OpaqueContext context;
    private transient LogEvent logEvent;
    private transient  ReferenceMetaData referenceMetaData;

    public CudaOpContextDeallocator(CudaOpContext ctx) {
        context = (OpaqueContext) ctx.contextPointer();
        if(EventLogger.getInstance().isEnabled()) {
            logEvent = LogEvent.builder()
                    .eventType(EventType.DEALLOCATION)
                    .objectAllocationType(ObjectAllocationType.OP_CONTEXT)
                    .associatedWorkspace(Nd4j.getWorkspaceManager().getWorkspaceForCurrentThread().getId())
                    .build();

        }

        referenceMetaData = ReferenceMetaData.builder().build();
    }

    @Override
    public void deallocate() {
        NativeOpsHolder.getInstance().getDeviceNativeOps().deleteGraphContext(context);
    }

    @Override
    public LogEvent logEvent() {
        return logEvent;
    }

    @Override
    public ReferenceMetaData referenceMetaData() {
        return referenceMetaData;
    }
}
