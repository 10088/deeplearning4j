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

//
// @author raver119@gmail.com, created  on 29/10/17.
// @author Yurii Shyrma (iuriish@yahoo.com), changed on 09.05.2018
//

#include <system/op_boilerplate.h>
#if NOT_EXCLUDED(OP_maxpool2d)

#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/helpers/convolutions.h>

namespace sd {
namespace ops {

//////////////////////////////////////////////////////////////////////////
// maxpool2d corresponds to poolingMode=0
CUSTOM_OP_IMPL(maxpool2d, 1, 1, false, 0, 9) {
  auto input = INPUT_VARIABLE(0);

  REQUIRE_TRUE(input->rankOf() == 4, 0, "MAXPOOL2D OP: input array should have rank of 4, but got %i instead",
               input->rankOf());

  // 0,1 - kernel Height/Width; 2,3 - stride Height/Width; 4,5 - pad Height/Width; 6,7 - dilation Height/Width; 8 - same
  // mode;
  auto output = OUTPUT_NULLIFIED(0);

  const int kH = INT_ARG(0);
  const int kW = INT_ARG(1);
  const int sH = INT_ARG(2);
  const int sW = INT_ARG(3);
  int pH = INT_ARG(4);
  int pW = INT_ARG(5);
  const int dH = INT_ARG(6);
  const int dW = INT_ARG(7);
  const bool isSameMode = INT_ARG(8);

  REQUIRE_TRUE(dH != 0 && dW != 0, 0, "MAXPOOL2D op: dilation must not be zero, but got instead {%i, %i}", dH, dW);

  int oH = 0;
  int oW = 0;

  int isNCHW = block.getIArguments()->size() > 10 ? !INT_ARG(10) : 1;  // INT_ARG(10): 1-NHWC, 0-NCHW

  const int iH = isNCHW ? input->sizeAt(2) : input->sizeAt(1);
  const int iW = isNCHW ? input->sizeAt(3) : input->sizeAt(2);

  if (!isNCHW) {
    input = new NDArray(input->permute({0, 3, 1, 2}));    // [bS, iH, iW, iC] -> [bS, iC, iH, iW]
    output = new NDArray(output->permute({0, 3, 1, 2}));  // [bS, oH, oW, iC] -> [bS, iC, oH, oW]
  }

  ConvolutionUtils::calcOutSizePool2D(oH, oW, kH, kW, sH, sW, pH, pW, dH, dW, iH, iW, isSameMode);

  if (isSameMode) ConvolutionUtils::calcPadding2D(pH, pW, oH, oW, iH, iW, kH, kW, sH, sW, dH, dW);

  // 0,1 - kernel Height/Width; 2,3 - stride Height/Width; 4,5 - pad Height/Width; 6,7 - dilation Height/Width;
  // poolingMode; 9 - divisor;
  ConvolutionUtils::pooling2d(block, *input, *output, kH, kW, sH, sW, pH, pW, dH, dW, PoolingType::MAX_POOL, 1);

  if (!isNCHW) {
    delete input;
    delete output;
  }

  return sd::Status::OK;
}

DECLARE_SYN(MaxPool2D, maxpool2d);
DECLARE_SYN(MaxPool, maxpool2d);
DECLARE_SYN(maxpool, maxpool2d);

DECLARE_TYPES(maxpool2d) { getOpDescriptor()->setAllowedInputTypes(sd::DataType::ANY)->setSameMode(true); }

DECLARE_SHAPE_FN(maxpool2d) {
  // NDArray<T> *x = block.getVariables().at(0)->getNDArray();
  auto inShape = inputShape->at(0);
  auto shapeOf = shape::shapeOf(inShape);
  // 0 - number of dimensions; 1,2 - kernel Height/Width; 3,4 - stride Height/Width; 5,6 - pad Height/Width; 7,8 -
  // dilation Height/Width; 9,10 - input Height/Width; 11 - batch size; 12 - input depth; 13 - same mode;
  int kH = INT_ARG(0);
  int kW = INT_ARG(1);
  int sH = INT_ARG(2);
  int sW = INT_ARG(3);
  int pH = INT_ARG(4);
  int pW = INT_ARG(5);
  int dH = INT_ARG(6);
  int dW = INT_ARG(7);
  int isSameMode = INT_ARG(8);
  int isNCHW = block.getIArguments()->size() > 10 ? !INT_ARG(10) : 1;  // INT_ARG(10): 1-NHWC, 0-NCHW

  REQUIRE_TRUE(dH != 0 && dW != 0, 0, "MAXPOOL2D op: dilation must not be zero, but got instead {%i, %i}", dH, dW);

  int bS = shapeOf[0];
  int iC = isNCHW ? shapeOf[1] : shapeOf[3];
  int iH = isNCHW ? shapeOf[2] : shapeOf[1];
  int iW = isNCHW ? shapeOf[3] : shapeOf[2];

  char order = shape::order(inShape);  // output order must be equal to input order

  // calculate output Height/Width
  int oH, oW;
  ConvolutionUtils::calcOutSizePool2D(oH, oW, kH, kW, sH, sW, pH, pW, dH, dW, iH, iW, isSameMode);

  // allocate memory for new shape
  sd::LongType newShape[4];

  newShape[0] = bS;
  if (isNCHW) {
    newShape[1] = iC;
    newShape[2] = oH;
    newShape[3] = oW;
  } else {
    newShape[1] = oH;
    newShape[2] = oW;
    newShape[3] = iC;
  }

  auto desc = new ShapeDescriptor(ArrayOptions::dataType(inShape), order, newShape, 4);
  auto ret =  SHAPELIST(ConstantShapeHelper::getInstance().createShapeInfo(desc));
  delete desc;
  return ret;
}

DECLARE_TYPES(maxpool2d_bp) {
  getOpDescriptor()->setAllowedInputTypes(sd::DataType::ANY)->setAllowedOutputTypes({ALL_FLOATS});
}

//////////////////////////////////////////////////////////////////////////
CUSTOM_OP_IMPL(maxpool2d_bp, 2, 1, false, 0, 10) {
  auto input = INPUT_VARIABLE(0);    // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW)
  auto gradO = INPUT_VARIABLE(1);    // [bS, oH, oW, oC] (NHWC) or [bS, oC, oH, oW] (NCHW), epsilon_next
  auto gradI = OUTPUT_NULLIFIED(0);  // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW), epsilon

  int kH = INT_ARG(0);                                                 // filter(kernel) height
  int kW = INT_ARG(1);                                                 // filter(kernel) width
  int sH = INT_ARG(2);                                                 // strides height
  int sW = INT_ARG(3);                                                 // strides width
  int pH = INT_ARG(4);                                                 // paddings height
  int pW = INT_ARG(5);                                                 // paddings width
  int dH = INT_ARG(6);                                                 // dilations height
  int dW = INT_ARG(7);                                                 // dilations width
  int isSameMode = INT_ARG(8);                                         // 0-VALID, 1-SAME
  int isNCHW = block.getIArguments()->size() > 10 ? !INT_ARG(10) : 1;  // INT_ARG(10): 1-NHWC, 0-NCHW

  REQUIRE_TRUE(input->rankOf() == 4, 0, "MAXPOOL2D_BP op: input should have rank of 4, but got %i instead",
               input->rankOf());
  REQUIRE_TRUE(dH != 0 && dW != 0, 0, "MAXPOOL2D_BP op: dilation must not be zero, but got instead {%i, %i}", dH, dW);

  int bS, iC, iH, iW, oC, oH,
      oW;  // batch size, input channels, input height/width, output channels, output height/width;
  int indIOioC, indIiH, indWoC, indWiC, indWkH, indOoH;  // corresponding indexes
  ConvolutionUtils::getSizesAndIndexesConv2d(isNCHW, 0, *input, *gradO, bS, iC, iH, iW, oC, oH, oW, indIOioC, indIiH,
                                             indWiC, indWoC, indWkH, indOoH);

  std::vector<sd::LongType> expectedGradOShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, iC, oH, oW, 0, indIOioC, indIiH, indIiH + 1});
  std::vector<sd::LongType> expectedGradIShape =
      ShapeUtils::composeShapeUsingDimsAndIdx({bS, iC, iH, iW, 0, indIOioC, indIiH, indIiH + 1});
  REQUIRE_TRUE(
      gradO->isSameShape(expectedGradOShape), 0,
      "MAXPOOL2D_BP op: wrong shape of output's gradients array (next epsilon), expected is %s, but got %s instead !",
      ShapeUtils::shapeAsString(expectedGradOShape).c_str(), ShapeUtils::shapeAsString(gradO).c_str());
  REQUIRE_TRUE(
      gradI->isSameShape(expectedGradIShape), 0,
      "MAXPOOL2D_BP op: wrong shape of input's gradients array (epsilon), expected is %s, but got %s instead !",
      ShapeUtils::shapeAsString(expectedGradIShape).c_str(), ShapeUtils::shapeAsString(gradI).c_str());

  if (!isNCHW) {
    input = new NDArray(input->permute({0, 3, 1, 2}));  // [bS, iH, iW, iC] -> [bS, iC, iH, iW]
    gradI = new NDArray(gradI->permute({0, 3, 1, 2}));  // [bS, iH, iW, iC] -> [bS, iC, iH, iW]
    gradO = new NDArray(gradO->permute({0, 3, 1, 2}));  // [bS, oH, oW, iC] -> [bS, iC, oH, oW]
  }

  if (isSameMode)  // SAME
    ConvolutionUtils::calcPadding2D(pH, pW, oH, oW, iH, iW, kH, kW, sH, sW, dH, dW);

  // NDArray<T> columnsWrongShape(input->ordering(), {bS, iC, oH, oW, kH, kW}, input->getWorkspace());
  // NDArray<T>* columns = columnsWrongShape.permute({0, 1, 4, 5, 2, 3});                                // [bS, iC, oH,
  // oW, kH, kW] -> [bS, iC, kH, kW, oH, oW]

  // input->template applyTransform<simdOps::Im2col<T>>(columns, std::vector<T>({(T)kH, (T)kW, (T)sH, (T)sW, (T)pH,
  // (T)pW, (T)dH, (T)dW, (T)0.f, (T)0.f}).data());

  // NDArray<T>* columns2d = columnsWrongShape.reshape('c', {bS*iC*oH*oW, kH*kW});
  // NDArray<T>* gradOVector = gradO->reshape('c', {(int) gradO->lengthOf(), 1});

  // columns2d->template applyTransform<simdOps::IsMax<T>>(std::vector<T>({(T)1., (T)1.}).data());
  // columns2d->muliColumnVector(gradOVector);

  // columns->template applyTransform<simdOps::Col2Im<T>>(gradI, std::vector<T>({(T)sH, (T)sW, (T)pH, (T)pW, (T)iH,
  // (T)iW, (T)dH, (T)dW}).data());

  ConvolutionUtils::pooling2dBP(block, *input, *gradO, *gradI, kH, kW, sH, sW, pH, pW, dH, dW, 0., 1.);

  if (!isNCHW) {
    delete input;
    delete gradI;
    delete gradO;
  }

  return sd::Status::OK;
}
DECLARE_SYN(MaxPool2D_bp, maxpool2d_bp);
DECLARE_SYN(MaxPool_bp, maxpool2d_bp);

DECLARE_SHAPE_FN(maxpool2d_bp) {
  REQUIRE_TRUE(inputShape->at(0)[0] == 4, 0, "MAXPOOL2D_BP op: input array must be 4D, but got %i instead!",
               inputShape->at(0)[0]);
  REQUIRE_TRUE(inputShape->at(1)[0] == 4, 0,
               "MAXPOOL2D_BP op: output's gradient array (next epsilon) must be 4D, but got %i instead!",
               inputShape->at(1)[0]);

  auto desc = new   ShapeDescriptor(inputShape->at(0), ArrayOptions::dataType(inputShape->at(1)));
  return SHAPELIST(ConstantShapeHelper::getInstance().createShapeInfo(desc));
}

}  // namespace ops
}  // namespace sd

#endif
